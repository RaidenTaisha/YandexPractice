#pragma once

#include <string>

class Person;

// Наблюдатель за состоянием человека.
class PersonObserver {
 public:
  // Этот метод вызывается, когда меняется состояние удовлетворённости человека
  virtual void OnSatisfactionChanged(Person& /*person*/, int /*old_value*/, int /*new_value*/) {}

 protected:
  // Класс PersonObserver не предназначен для удаления напрямую
  virtual ~PersonObserver() = default;
};

/*
    Человек.
    При изменении уровня удовлетворённости уведомляет
    связанного с ним наблюдателя
*/
class Person {
 public:
  Person(const std::string& name, int age) : name_(name), age_(age) {}
  virtual ~Person() = default;

  int GetSatisfaction() const {
    return satisfaction_;
  }

  const std::string& GetName() const {
    return name_;
  }

  // «Привязывает» наблюдателя к человеку. Привязанный наблюдатель
  // уведомляется об изменении уровня удовлетворённости человека
  // Новый наблюдатель заменяет собой ранее привязанного
  // Если передать nullptr в качестве наблюдателя, это эквивалентно отсутствию наблюдателя
  void SetObserver(PersonObserver *observer) {
    observer_ = observer;
  }

  int GetAge() const {
    return age_;
  }

  // Увеличивает на 1 количество походов на танцы
  // Увеличивает удовлетворённость на 1
  void Dance() {
    ++dance_count_;
    AfterDance();
  }

  int GetDanceCount() const {
    return dance_count_;
  }

  // Прожить день. Реализация в базовом классе ничего не делает
  virtual void LiveADay() {}

 protected:
  virtual void AfterDance() {
    SetSatisfaction(satisfaction_ + 1);
  }

  void SetSatisfaction(int value) {
    if (satisfaction_ != value) {
      int old_satisfaction = satisfaction_;
      satisfaction_ = value;
      if (observer_)
        observer_->OnSatisfactionChanged(*this, old_satisfaction, satisfaction_);
    }
  }

  int dance_count_ = 0;

 private:
  PersonObserver *observer_ = nullptr;
  std::string name_;
  int age_ = 0;
  int satisfaction_ = 100;
};

// Рабочий.
// День рабочего проходит за работой
class Worker final : public Person {
 public:
  Worker(const std::string& name, int age) : Person(name, age) {}

  void LiveADay() override {
    Work();
  }

  // Увеличивает счётчик сделанной работы на 1, уменьшает удовлетворённость на 5
  void Work() {
    ++done_work_count_;
    SetSatisfaction(GetSatisfaction() - 5);
  }

  // Возвращает значение счётчика сделанной работы
  int GetWorkDone() const {
    return done_work_count_;
  }

 protected:
  void AfterDance() override {
    if (const int age = GetAge(); age > 30 && age < 40) {
      SetSatisfaction(GetSatisfaction() + 2);
    } else {
      SetSatisfaction(GetSatisfaction() + 1);
    }
  }

 private:
  int done_work_count_ = 0;
};

// Студент.
// День студента проходит за учёбой
class Student final : public Person {
 public:
  Student(const std::string& name, int age) : Person(name, age) {}

  void LiveADay() override {
    Study();
  }

  // Учёба увеличивает уровень знаний на 1, уменьшает уровень удовлетворённости на 3
  void Study() {
    ++knowladge_count_;
    SetSatisfaction(GetSatisfaction() - 3);
  }

  // Возвращает уровень знаний
  int GetKnowledgeLevel() const {
    return knowladge_count_;
  }

 private:
  int knowladge_count_ = 0;
};
