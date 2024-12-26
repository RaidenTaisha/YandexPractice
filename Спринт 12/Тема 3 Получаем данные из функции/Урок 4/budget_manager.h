#pragma once

#include "date.h"

#include <vector>

struct DayState {
  [[nodiscard]] double ComputeIncome() const {
    return income - spent;
  }

  double income = 0;
  double spent = 0;
};

class BudgetManager {
 public:
  static const Date START_DATE;
  static const Date END_DATE;

  static int GetDayIndex(const Date& day) {
    return Date::ComputeDistance(START_DATE, day);
  }

  DayState& GetDayState(int index){
    return days_[index];
  }

  [[nodiscard]] const DayState& GetDayState(int index) const {
    return days_[index];
  }

 private:
  std::vector<DayState> days_ = std::vector<DayState>(GetDayIndex(END_DATE));
};
