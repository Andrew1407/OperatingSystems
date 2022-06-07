// #include "2-lab.cpp"
// #include "2-lab.threads.cpp"
#include "2-lab.async.cpp"

#include <ctime>

class Timer {
  std::clock_t begin;
  public:
    Timer(): begin(std::clock()) {}
    ~Timer() {
      std::clock_t end = std::clock();
      printf("\n\nFinished: %f.\n", double(end - begin) / CLOCKS_PER_SEC);
    }  
};

int main() {
  Allocator a;
  Timer t;
  void* b = a.allocateMemory(60);
  void* c = a.allocateMemory(61);
  void* d = a.allocateMemory(61);
  a.reallocateMemory(d, 50, 200);
  a.freeMemory(c, 58);  
  a.freeBlocksDump();

  return 0;
}
