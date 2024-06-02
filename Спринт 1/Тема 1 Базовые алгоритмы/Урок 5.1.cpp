#include <iostream>
#include <vector>
#include <numeric>

using namespace std;

double Average(const vector<int>& xs) {
  double ret = 0.0;
  if (!xs.empty()) {
    double sum = accumulate(xs.begin(), xs.end(), 0.0);
    ret = sum / xs.size();
  }
  return ret;
}

int main() {
  cout << Average({1, 2, 2}) << endl;
  return 0;
}
