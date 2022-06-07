#include <iostream>
#include <chrono>

using namespace std;

#define ARR_SIZE 100

void generate(int ***arr) {
  for (int j = 0; j < ARR_SIZE; j++) {
    arr[j] = new int*[ARR_SIZE];
    for (int i = 0; i < ARR_SIZE; i++)
      arr[j][i] = new int[ARR_SIZE];
  }
}

void clear(int ***arr) {
  for (int j = 0; j < ARR_SIZE; j++) {
    for (int i = 0; i < ARR_SIZE; i++)
      delete[] arr[j][i];
    delete[] arr[j];
  }
  delete[] arr;
}

int64_t calcTime(void (&fn)(int***)) {
  int ***arr = new int**[ARR_SIZE];
  generate(arr);
  auto start = chrono::high_resolution_clock::now();
  fn(arr);
  auto end = chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<chrono::microseconds>(end - start);
  clear(arr);
  return duration.count();
}

void unoptimized(int ***arr) {
  for (int j = 0; j < ARR_SIZE; j++)
    for (int i = 0; i < ARR_SIZE; i++)
      for (int k = 0; k < ARR_SIZE; k++)
        arr[k][j][i]++;
}

void optimized(int ***arr) {
  for (int j = 0; j < ARR_SIZE; j++)
    for (int i = 0; i < ARR_SIZE; i++)
      for (int k = 0; k < ARR_SIZE; k++)
        arr[j][i][k]++;
}

int main() {
  const int64_t unopRes = calcTime(unoptimized);
  const int64_t opRes = calcTime(optimized);
  const double ratio = (float)unopRes / (float)opRes;

  cout << "Unoptimized execution time: " << unopRes << " mircosec.\n";
  cout << "Optimized execution time: " << opRes << " mircosec.\n";
  cout << "Difference: " << unopRes - opRes << " mircosec.\n";
  cout << "Ratio: " << ratio << " mircosec.\n";

  return 0;
}
