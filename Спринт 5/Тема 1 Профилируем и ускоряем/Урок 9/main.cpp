#include "log_duration.h"

#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

using namespace std;

vector<float> ComputeAvgTemp(const vector<vector<float>>& measures) {
  const int num_days = measures.size();
  if (measures.empty()) return {};
  const int measures_per_day = measures[0].size();

  vector<float> results(measures_per_day);
  vector<float> sums(measures_per_day, 0.f);
  vector<int> counts(measures_per_day, 0);

  for (int j = 0; j < num_days; ++j) {
    for (int i = 0; i < measures_per_day; ++i) {
      float c = measures[j][i];
      sums[i] += (c > 0 ? c : 0.f);
      counts[i] += (c > 0 ? 1 : 0);
    }
  }

  for (int i = 0; i < measures_per_day; ++i) {
    results[i] = counts[i] == 0 ? 0 : sums[i] / counts[i];
  }

  return results;
}

vector<float> GetRandomVector(int size) {
  static mt19937 engine;
  uniform_real_distribution<float> d(-100, 100);

  vector<float> res(size);
  for (int i = 0; i < size; ++i) {
    res[i] = d(engine);
  }

  return res;
}

void Operate() {
  vector<vector<float>> data;
  data.reserve(5000);

  for (int i = 0; i < 5000; ++i) {
    data.push_back(GetRandomVector(5000));
  }

  vector<float> avg;
  {
    LOG_DURATION("ComputeAvgTemp"s);
    avg = ComputeAvgTemp(data);
  }

  cout << "Total mean: "s << accumulate(avg.begin(), avg.end(), 0.f) / avg.size() << endl;
}

int main() {
  Operate();
}