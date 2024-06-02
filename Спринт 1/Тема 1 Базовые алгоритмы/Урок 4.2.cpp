#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

pair<bool, double> CalcMedian(vector<double>& elems) {
  pair<bool, double> ret { false, 0.0 };
  if (!elems.empty()) {
    auto m = elems.begin() + elems.size() / 2;
    nth_element(elems.begin(), m, elems.end());
    int center = elems.size() / 2;
    if (elems.size() % 2 == 0) {
      double median = (elems[center] + elems[center - 1]) / 2;
      ret = { true, median };
    } else {
      double median = elems[center];
      ret = { true, median };
    }
    cout << ret.second << endl;
  } else {
    cout << "Empty vector" << endl;
  }
  return ret;
}

int main() {
  int count;
  cin >> count;
  vector<double> elems;
  for (int i = 0; i < count; ++i) {
    double elem;
    cin >> elem;
    elems.push_back(elem);
  }
  CalcMedian(elems);
}
