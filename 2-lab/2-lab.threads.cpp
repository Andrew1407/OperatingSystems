#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>
#include <map>
#include <vector>
#include <math.h>
#include <thread>
#include <mutex>
#include <condition_variable>

#define MIN_SIZE 4
#define DEFAULT_SIZE 1 << 11

typedef char* block_addr;
typedef std::vector<block_addr>  addrs_vector;
typedef std::map<size_t, addrs_vector> blocks_map;


class Allocator {
  private:
    blocks_map _freeBlocks;
    size_t _allocatedSize;
    block_addr _startPtr, _endPtr;
    std::mutex _mtx;
    std::condition_variable _cv;
    bool _finished;               //for treads awaiting

    size_t _align(const size_t size) {
      return std::ceil(std::log2(size));
    }

    bool _impaled(const block_addr blockPtr) {
      return blockPtr < _startPtr || blockPtr > (_endPtr - MIN_SIZE);
    }

    void _buddyDivision(block_addr blockPtr, const size_t size, const size_t foundSize) {
      size_t insertedSize = foundSize;
      while (insertedSize != size)
        if (_freeBlocks.find(--insertedSize) == _freeBlocks.end()) {
          addrs_vector buddyClass = { blockPtr + (1 << insertedSize) };
          _freeBlocks.emplace(insertedSize, buddyClass);
        } else {
          _freeBlocks[insertedSize].push_back(blockPtr);
        }
    }

    void* _reserveBlock(const size_t size) {
      blocks_map::iterator foundClass = _freeBlocks.begin();
      while (foundClass != _freeBlocks.end())
        if (foundClass->first >= size && foundClass->second.size()) break;
        else foundClass++;
      if (
        foundClass == _freeBlocks.end() ||
        foundClass->first < size ||
        !foundClass->second.size()
      ) return nullptr;
      block_addr reservedPtr = foundClass->second.back();
      foundClass->second.pop_back();
      if (foundClass->first > size)
        _buddyDivision(reservedPtr, size, foundClass->first);
      return (void*)reservedPtr;
    }

    void _coalesce(block_addr blockPtr, const size_t blockSize) {
      addrs_vector* sizeClass = &_freeBlocks[blockSize];
      if (!sizeClass->size()) {
        sizeClass->push_back(blockPtr);
        return;
      }
      addrs_vector::iterator coalesced = sizeClass->begin();
      const size_t buddySize = 1 << blockSize;
      while (coalesced != sizeClass->end())
        if (
          blockPtr + buddySize == *coalesced ||
          blockPtr - buddySize == *coalesced
        ) break;
        else coalesced++;
      if (coalesced == sizeClass->end()) {
        sizeClass->push_back(blockPtr);
        return;
      }
      block_addr coalescedAddr = std::min(blockPtr, *coalesced);
      sizeClass->erase(coalesced);
      _coalesce(coalescedAddr, blockSize + 1);
    }

  public:
    Allocator(const size_t size = DEFAULT_SIZE) {
      size_t aligned = size > 0 ? _align(size) : MIN_SIZE;
      void* allocatedMem = mmap(0, (1 << aligned), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
      if (allocatedMem == MAP_FAILED) {
        allocatedMem = mmap(0, DEFAULT_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        aligned = 11;
      }
      addrs_vector inited = { (block_addr)allocatedMem };
      _freeBlocks.emplace(aligned, inited);
      _startPtr = (block_addr)allocatedMem;
      _allocatedSize = 1 << aligned;
      _endPtr = (block_addr)allocatedMem + _allocatedSize;
    }

    void* allocateMemory(const size_t size) {
      if (size <= 0 || size > _allocatedSize)
        return nullptr;
      const size_t aligned = size < MIN_SIZE ? 2 : _align(size);
      return _reserveBlock(aligned);
    }

    void freeMemory(void* &blockPtr, const size_t blockSize) {
      if (
        !blockPtr ||
        _impaled((block_addr)blockPtr) ||
        blockSize < MIN_SIZE ||
        blockSize > _allocatedSize
      ) return;
      _finished = false;
      std::thread coalescing([&] {
        std::unique_lock<std::mutex> ul(_mtx);
        _coalesce((block_addr)blockPtr, _align(blockSize));
        _finished = true;
        ul.unlock();
        _cv.notify_one();
      });
      {
        std::unique_lock<std::mutex> ul(_mtx);
        _cv.wait(ul, [&] { return _finished; });
      }
      blockPtr = nullptr;
      coalescing.join();
    }

    void* reallocateMemory(void* &blockPtr, const size_t blockSize, const size_t resized) {
      if (
        blockPtr && _impaled((block_addr)blockPtr) ||
        resized < MIN_SIZE ||
        resized > _allocatedSize ||
        resized - blockSize < MIN_SIZE ||
        resized - blockSize > _allocatedSize
      ) return nullptr;
      if (!blockPtr) {
        blockPtr = allocateMemory(resized);
        return blockPtr;
      }
      std::thread freeing([&] {
        freeMemory(blockPtr, blockSize);
      });
      blockPtr = allocateMemory(resized);
      freeing.join();
      return blockPtr;
    }

    void freeBlocksDump() {
      size_t totalSizeAll = 0;
      for (auto blockClass : _freeBlocks) {
        const size_t classSize = 1 << blockClass.first;
        printf("Class %d (%d bytes):\n", blockClass.first, classSize);
        size_t totalSizeClass = 0;
        if (blockClass.second.size()) {
          for (block_addr blockPtr : blockClass.second) {
            printf("  %p\n", blockPtr);
            totalSizeClass++;
          }
          const size_t sizeFree = classSize * totalSizeClass;
          printf("Total size for %d class (%d bytes): %d bytes.\n\n", blockClass.first, classSize,  sizeFree);
          totalSizeAll += sizeFree;
        }
        else {
          printf("  EMPTY\n\n");
        }
      }
      printf("Totally free: %d bytes.\n", totalSizeAll);
      printf("Totally taken: %d bytes.\n", _allocatedSize - totalSizeAll);
    }

    ~Allocator() {
      munmap(_startPtr, _allocatedSize);
    }
};
