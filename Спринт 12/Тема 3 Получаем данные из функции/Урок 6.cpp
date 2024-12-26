#include <algorithm>
#include <numeric>
#include <cassert>
#include <optional>
#include <utility>
#include <vector>

using namespace std;

// напишите функцию ComputeStatistics, принимающую 5 параметров:
// два итератора, выходное значение для суммы, суммы квадратов и максимального элемента
template<typename Iterator, typename OutSum, typename OutSqSum, typename OutMax>
void ComputeStatistics(Iterator begin,
                       Iterator end,
                       OutSum &sum,
                       OutSqSum &sum_of_squares,
                       OutMax &max_element) {
  using Type = std::decay_t<decltype(*begin)>;
  optional<Type> SUM = nullopt;
  optional<Type> SUM_OF_SQUARES = nullopt;
  optional<Type> MAX_ELEMENT = nullopt;
  constexpr bool need_sum = !is_same_v<OutSum, const nullopt_t>;
  constexpr bool need_sq_sum = !is_same_v<OutSqSum, const nullopt_t>;
  constexpr bool need_max = !is_same_v<OutMax, const nullopt_t>;
  while (begin != end) {
    if constexpr (need_sum) {
      if (!SUM) {
        SUM = *begin;
      } else {
        *SUM += *begin;
      }
    }
    if constexpr (need_sq_sum) {
      if (!SUM_OF_SQUARES) {
        SUM_OF_SQUARES = *begin * *begin;
      } else {
        *SUM_OF_SQUARES += *begin * *begin;
      }
    }
    if constexpr (need_max) {
      if (!MAX_ELEMENT) {
        MAX_ELEMENT = *begin;
      } else {
        if (MAX_ELEMENT < *begin)
          *MAX_ELEMENT = *begin;
      }
    }
    ++begin;
  }
  if constexpr (need_sum) { sum = *SUM; }
  if constexpr (need_sq_sum) { sum_of_squares = *SUM_OF_SQUARES; }
  if constexpr (need_max) { max_element = *MAX_ELEMENT; }
}

struct OnlySum {
  int value;
};

OnlySum operator+(OnlySum l, OnlySum r) {
  return {l.value + r.value};
}
OnlySum &operator+=(OnlySum &l, OnlySum r) {
  return l = l + r;
}

int main() {
  vector input = {1, 2, 3, 4, 5, 6};
  int sq_sum = 0;
  std::optional<int> max;

  // Переданы выходные параметры разных типов - std::nullopt_t, int и std::optional<int>
  ComputeStatistics(input.begin(), input.end(), nullopt, sq_sum, max);

  assert(sq_sum == 91 && max && *max == 6);

  vector<OnlySum> only_sum_vector = {{100}, {-100}, {20}};
  OnlySum sum{};

  // Поданы значения поддерживающие только суммирование, но запрошена только сумма
  ComputeStatistics(only_sum_vector.begin(),
                    only_sum_vector.end(),
                    sum,
                    nullopt,
                    nullopt);

  assert(sum.value == 20);
}
