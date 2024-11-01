#pragma once

#include "people.h"

/*
    Надзиратель за уровнем удовлетворённости.
    Способен наблюдать за состоянием человека
    Если уровень удовлетворённости человека опустится ниже минимального уровня, Надзиратель
    побуждает человека танцевать до тех пор, пока уровень удовлетворённости
    не станет больше или равен максимальному значению
*/
class SatisfactionSupervisor : public PersonObserver {
 public:
  // Сохраняем верхний и нижний уровни удовлетворённости
  SatisfactionSupervisor(int min_satisfaction, int max_satisfaction)
      : min_satisfaction_(min_satisfaction), max_satisfaction_(max_satisfaction) {}

  void OnSatisfactionChanged(Person& person, int old_value, int new_value) override {
    // Если уровень удовлетворённости опустился ниже нижней границы
    if (new_value < old_value && new_value < min_satisfaction_) {
      // Побуждаем человека танцевать, пока уровень удовлетворённости не поднимется до верхнего уровня
      while (person.GetSatisfaction() < max_satisfaction_) {
        person.Dance();
      }
    }
  }

 private:
  int min_satisfaction_;
  int max_satisfaction_;
};
