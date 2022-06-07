#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>

#define MIN_PAYLOAD 4
#define HEADER_SIZE 8
#define MIN_BLOCK_SIZE 12

class Allocator {
  void *_startPtr, *_endPtr;
  size_t _allocSize;
  typedef struct { int current, previous; } header;

  size_t _alignSize(const size_t size) {
    return (size < MIN_BLOCK_SIZE) ? MIN_BLOCK_SIZE :
      (size % MIN_PAYLOAD) ? ((size / MIN_PAYLOAD + 1) * MIN_PAYLOAD) : size;
  }

  header _mkHeader(const size_t crtSize, const bool crtAlloc, const size_t pvsSize, const bool pvsAlloc) {
    const int current = _alignSize(crtSize) | crtAlloc;
    const int previous = pvsSize ?
      (_alignSize(pvsSize) | pvsAlloc) : 0;
    return { current, previous };
  }

  header _mkHeader(const size_t crtSize, const bool crtAlloc, const int previous) {
    const int current = _alignSize(crtSize) | crtAlloc;
    return { current, previous };
  }

  size_t _getSize(const int blockDesc) {
    return blockDesc & ~0x3;
  }

  bool _getAlloc(const int blockDesc) {
    return blockDesc & 0b1;
  }

  void* _reserveBlock(const void* blockPtr, const size_t reservedSize) {
    const size_t blockSize = _getSize(((header*)blockPtr)->current);
    void* nextPtr = (char*)blockPtr +_getSize(((header*)blockPtr)->current);
    const size_t allocSize = blockSize - reservedSize < MIN_BLOCK_SIZE ?
      blockSize : reservedSize;
    *(header*)(blockPtr) =
      _mkHeader(allocSize, true, ((header*)blockPtr)->previous);    
    if (allocSize != blockSize) {
      const size_t freeSize = blockSize - allocSize;
      *(header*)((char*)blockPtr + allocSize) =
        _mkHeader(freeSize, false, ((header*)blockPtr)->current);
      const bool nextAlloc = _getAlloc(((header*)nextPtr)->current);
      const size_t nextSize = _getSize(((header*)nextPtr)->current);
      if (nextAlloc) {
        *(header*)nextPtr = _mkHeader(nextSize, nextAlloc, freeSize, false);
      } else {
        nextPtr = ((char*)blockPtr + allocSize);
        *(header*)nextPtr = _mkHeader(freeSize + nextSize, false, ((header*)blockPtr)->current);
      }
    }
    return (void*)((char*)blockPtr + HEADER_SIZE);
  }

  public:
    Allocator(): _startPtr(nullptr), _endPtr(nullptr), _allocSize(0) {}

    bool initMemory(const size_t size = 1 << 12) {
      if (size <= 0)
        return false;
      const size_t sizeMap = _alignSize(size + HEADER_SIZE);
      void* allocatedMem = mmap(0, sizeMap, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
      if (allocatedMem == MAP_FAILED)
        return false;
      const header initialHeader = _mkHeader(sizeMap, false, 0);
      *(header*)allocatedMem = initialHeader;
      _allocSize = sizeMap;
      _startPtr = (void *)allocatedMem;
      _endPtr = (void *)((size_t)allocatedMem + sizeMap);
      return true;
    }

    void* allocateMemory(const size_t size) {
      if (size <= 0)
        return nullptr;
      const size_t alignedSize = _alignSize(size + HEADER_SIZE);
      if (alignedSize > _allocSize)
        return nullptr;
      void* properBlock = nullptr;
      for (void* blockPtr = (void*)_startPtr; blockPtr < _endPtr;) {
        const bool alloc = _getAlloc(((header*)blockPtr)->current);
        const size_t blockSize = _getSize(((header*)blockPtr)->current);
        const size_t properSize = properBlock ?
          _getSize(((header*)properBlock)->current) : 0;
        if (!alloc && blockSize >= alignedSize)
          if ((properSize > blockSize) && properBlock || !properBlock)
            properBlock = blockPtr;
        blockPtr = (void*)((char*)blockPtr + blockSize);
      }
      return properBlock ? _reserveBlock(properBlock, alignedSize) : nullptr;
    }

    void* reallocateMemory (void* &payloadPtr, const size_t newSize) {
      void* blockPtr = (void*)((char*)payloadPtr - HEADER_SIZE);
      if (
        blockPtr < _startPtr ||
        blockPtr > ((char*)_endPtr - MIN_BLOCK_SIZE) ||
        newSize < MIN_PAYLOAD
      )
        return nullptr;
      if (!blockPtr)
        return allocateMemory(newSize);
      const size_t resized = _alignSize(newSize + HEADER_SIZE);
      const size_t blockSize = _getSize(((header*)blockPtr)->current);
      if (blockSize == resized || resized >= _allocSize)
        return nullptr;
      freeMemory(payloadPtr);
      payloadPtr = allocateMemory(newSize);
      return payloadPtr;
    }

    void freeMemory(void* payloadPtr) {
      if (!payloadPtr) return;
      void* blockPtr = (void*)((char*)payloadPtr - HEADER_SIZE);
      const size_t blockSize = _getSize(((header*)blockPtr)->current);
      bool modified = false;
      const int prvDesc = ((header*)blockPtr)->previous;
      if (prvDesc && !_getAlloc(prvDesc)) {
        const int prvSize = _getSize(prvDesc);
        blockPtr = (void*)((char*)blockPtr - prvSize);
        *(header*)blockPtr = _mkHeader(prvSize + blockSize, false, prvDesc);
        modified = true;
      }
      const void* nextPtr = (void*)((char*)blockPtr + _getSize(((header*)blockPtr)->current));
      const int nextDesc = nextPtr >= _endPtr ?
        1 : ((header*)nextPtr)->current;
      if (!_getAlloc(nextDesc)) {
        *(header*)blockPtr =
          _mkHeader(_getSize(((header*)blockPtr)->current) + _getSize(nextDesc), false, ((header*)blockPtr)->previous);
        modified = true;
      } else {
        *(header*)nextPtr =
          _mkHeader(_getSize(nextDesc), true, _getSize(((header*)blockPtr)->current), false);
      }
      if (!modified)
        *(header*)blockPtr = _mkHeader(blockSize, false, ((header*)blockPtr)->previous);
    }

    void memoryDump() {
      for (void* blockPtr = (void*)_startPtr; blockPtr < _endPtr;) {
        const size_t size = _getSize(((header*)blockPtr)->current);
        if (!size) return;
        const bool alloc = _getAlloc(((header*)blockPtr)->current);
        printf("Address: %p;\nTotal block size: %d bytes;\nPayload: %d bytes;\nFree: %s;\n\n", blockPtr, size, size - HEADER_SIZE, alloc ? "no" : "yes");
        blockPtr = (void*)((char*)blockPtr + size);
      }
    }

    ~Allocator() {
      if (_startPtr) {
        munmap(_startPtr, _allocSize);
        _startPtr = _endPtr = nullptr;
      }
    }
};
