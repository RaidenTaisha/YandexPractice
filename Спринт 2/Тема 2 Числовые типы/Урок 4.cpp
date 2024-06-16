#include <cstdint>
#include <iostream>
#include <limits>

using namespace std;

int main() {
  const auto min = numeric_limits<int64_t>::min();
  const auto max = numeric_limits<uint64_t>::max();
  cout << min << "\n"
       << max << "\n"
       << min + max << "\n"
       << min * 2 << "\n"
       << max * 2 << "\n";
  return 0;
}
