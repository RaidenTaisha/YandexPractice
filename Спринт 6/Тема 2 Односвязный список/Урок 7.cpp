#include <cassert>
#include <vector>
#include <algorithm>

template <typename T>
void ReverseArray(T* start, size_t size) {
  auto* end = start;
  for(size_t i = 0; i < size; i++) {
    end++;
  }
  std::reverse(start, end);
}

int main() {
  using namespace std;

  vector<int> v = {1, 2};
  ReverseArray(v.data(), v.size());
  assert(v == (vector<int>{2, 1}));
}
