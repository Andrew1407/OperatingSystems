#include "1-lab.cpp"


// int main() {
//   Allocator a = Allocator();
//   const bool inited = a.initMemory(200);
//   if (!inited) return 0;
//   void* p = a.allocateMemory(32);
//   void* b =a.allocateMemory(8);
//   // a.reallocateMemory(p, 8);
//   a.freeMemory(b);
//   a.memoryDump();
//   return 0;
// }

int main() {
  Allocator a = Allocator();
  const bool inited = a.initMemory(210);
  if (!inited) return 0;
  void* p = a.allocateMemory(32);
  void* b = a.allocateMemory(18);
  void* q = a.allocateMemory(26);
  // a.memoryDump();
  a.reallocateMemory(b, 100);
  printf("===============================\n");
  a.freeMemory(q);
  // a.memoryDump();
  a.freeMemory(b);
  // a.freeMemory(p);
  printf("===============================\n");
  a.memoryDump();
  return 0;
}
