#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

enum class Gender { FEMALE, MALE };

struct Person {
  int age;           // возраст
  Gender gender;     // пол
  bool is_employed;  // имеет ли работу
};

template<typename InputIt>
int ComputeMedianAge(InputIt range_begin, InputIt range_end) {
  if (range_begin == range_end) {
    return 0;
  }
  vector<typename InputIt::value_type> range_copy(range_begin, range_end);
  auto middle = range_copy.begin() + range_copy.size() / 2;
  nth_element(range_copy.begin(),
              middle,
              range_copy.end(),
              [](const Person &lhs, const Person &rhs) {
                return lhs.age < rhs.age;
              });
  return middle->age;
}

// напишите сигнатуру и реализацию функции PrintStats
void PrintStats(vector<Person> persons) {
  // Преобразуем порядок людей к следующему:
  //                  persons
  //                 /      \
  //          females        males
  //         /       \     /      \
  //      empl.  unempl. empl.   unempl.
  auto females_end =
      partition(persons.begin(), persons.end(), [](const Person &p) {
        return p.gender == Gender::FEMALE;
      });
  auto employed_females_end =
      partition(persons.begin(), females_end, [](const Person &p) {
        return p.is_employed;
      });
  auto employed_males_end =
      partition(females_end, persons.end(), [](const Person &p) {
        return p.is_employed;
      });

  // Теперь интересующие нас группы находятся в векторе непрерывно
  cout << "Median age = "s << ComputeMedianAge(persons.begin(), persons.end())
       << endl;
  cout << "Median age for females = "s
       << ComputeMedianAge(persons.begin(), females_end) << endl;
  cout << "Median age for males = "s
       << ComputeMedianAge(females_end, persons.end()) << endl;
  cout << "Median age for employed females = "s
       << ComputeMedianAge(begin(persons), employed_females_end) << endl;
  cout << "Median age for unemployed females = "s
       << ComputeMedianAge(employed_females_end, females_end) << endl;
  cout << "Median age for employed males = "
       << ComputeMedianAge(females_end, employed_males_end) << endl;
  cout << "Median age for unemployed males = "s
       << ComputeMedianAge(employed_males_end, persons.end()) << endl;
}

int main() {
  vector<Person> persons = {
      {31, Gender::MALE, false}, {40, Gender::FEMALE, true},
      {24, Gender::MALE, true}, {20, Gender::FEMALE, true},
      {80, Gender::FEMALE, false}, {78, Gender::MALE, false},
      {10, Gender::FEMALE, false}, {55, Gender::MALE, true},
  };
  PrintStats(persons);
}
