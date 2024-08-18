#include <chrono>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <vector>

#include "log_duration.h"

using namespace std;
using namespace literals;

vector<int> ReverseVector(const vector<int>& source_vector) {
  vector<int> res;
  for (int i : source_vector) {
    res.insert(res.begin(), i);
  }

  return res;
}

int CountPops(const vector<int>& source_vector, int begin, int end) {
  int res = 0;

  for (int i = begin; i < end; ++i) {
    if (source_vector[i]) {
      ++res;
    }
  }

  return res;
}

void AppendRandom(vector<int>& v, int n) {
  for (int i = 0; i < n; ++i) {
    // получаем случайное число с помощью функции rand.
    // с помощью (rand() % 2) получим целое число в диапазоне 0..1.
    // в C++ имеются более современные генераторы случайных чисел,
    // но в данном уроке не будем их касаться
    v.push_back(rand() % 2);
  }
}

void Operate() {
  LogDuration guard("Total"s);
  vector<int> random_bits;

  // операция << для целых чисел это сдвиг всех бит в двоичной
  // записи числа. Запишем с её помощью число 2 в степени 17 (131072)
  static const int N = 1 << 17;

  {
    // заполним вектор случайными числами 0 и 1
    LogDuration append("Append random"s);
    AppendRandom(random_bits, N);
  }

  vector<int> reversed_bits;
  {
    LogDuration reverse("Reverse"s);
    // перевернём вектор задом наперёд
    reversed_bits = ReverseVector(random_bits);
  }

  {
    LogDuration counting_timer("Counting"s);
    // посчитаем процент единиц на начальных отрезках вектора
    for (int i = 1, step = 1; i <= N; i += step, step *= 2) {
      double rate = CountPops(reversed_bits, 0, i) * 100. / i;
      cout << "After "s << i << " bits we found "s << rate << "% pops"s << endl;
    }
  }
}

int main() {
  // С спецификатором const LogDuration тесты не проходят
  Operate();
}
