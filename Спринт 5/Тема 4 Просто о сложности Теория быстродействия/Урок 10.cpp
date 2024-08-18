#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

using namespace std;

int EffectiveCount(const vector<int>& v, int n, int i) {
  const auto predict_value = static_cast<int64_t>(v.size()) * (i + 1) / (n + 1);
  if (log2(v.size()) > predict_value) {
    cout << "Using find_if" << endl;
    const auto& iter = find_if(v.begin(), v.end(), [i](const int num){
      return num > i;
    });
    return distance(v.begin(), iter);
  } else {
    cout << "Using upper_bound" << endl;
    const auto& iter = upper_bound(v.begin(), v.end(), ++i);
    return distance(v.begin(), iter);
  }
}

int main() {
  static const int NUMBERS = 1'000'000;
  static const int MAX = 1'000'000'000;

  mt19937 r;
  uniform_int_distribution<int> uniform_dist(0, MAX);

  vector<int> nums;
  for (int i = 0; i < NUMBERS; ++i) {
    int random_number = uniform_dist(r);
    nums.push_back(random_number);
  }
  sort(nums.begin(), nums.end());

  int i;
  cin >> i;
  int result = EffectiveCount(nums, MAX, i);
  cout << "Total numbers before "s << i << ": "s << result << endl;
}
