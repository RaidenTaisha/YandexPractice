#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <utility>

using namespace std;

template<typename Element>
ostream &operator<<(ostream &out, const vector<Element> &container);
template<typename Element>
ostream &operator<<(ostream &out, const set<Element> &container);
template<typename Key, typename Value>
ostream &operator<<(ostream &out, const map<Key, Value> &container);

template<class Key, class Value>
ostream &operator<<(ostream &out, const pair<Key, Value> &element) {
  out << "(";
  out << element.first << ", " << element.second;
  out << ")";
  return out;
}

template<class Container>
void Print(ostream &out, const Container &container) {
  using namespace string_literals;
  bool is_first = true;
  for (const auto &element : container) {
    if (is_first) {
      out << element;
      is_first = false;
    } else {
      out << ", "s << element;
    }
  }
}

template<class Element>
ostream &operator<<(ostream &out, const vector<Element> &container) {
  out << "[";
  Print(out, container);
  out << "]";
  return out;
}

template<class Element>
ostream &operator<<(ostream &out, const set<Element> &container) {
  out << "{";
  Print(out, container);
  out << "}";
  return out;
}

template<class Key, class Value>
ostream &operator<<(ostream &out, const map<Key, Value> &container) {
  out << "<";
  Print(out, container);
  out << ">";
  return out;
}

int main() {
  const set<string> cats = {"Мурка"s, "Белка"s, "Георгий"s, "Рюрик"s};
  cout << cats << endl;
  const vector<int> ages = {10, 5, 2, 12};
  cout << ages << endl;
  const map<string, int> cat_ages = {
      {"Мурка"s, 10},
      {"Белка"s, 5},
      {"Георгий"s, 2},
      {"Рюрик"s, 12}
  };
  cout << cat_ages << endl;
  return 0;
}
