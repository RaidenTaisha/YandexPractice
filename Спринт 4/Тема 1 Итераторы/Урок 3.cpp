#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <vector>

using namespace std;

template<class T>
void
PrintRange(T begin, T end) {
  while (begin != end) {
    cout << *begin++ << ' ';
  }
  cout << endl;
}

template<class Container, class Obj>
void
FindAndPrint(const Container &container, Obj obj) {
  auto it = find(container.begin(), container.end(), obj);
  PrintRange(container.begin(), it);
  PrintRange(it, container.end());
}

int main() {
  set<int> test = {1, 1, 1, 2, 3, 4, 5, 5};
  cout << "Test1"s << endl;
  FindAndPrint(test, 3);
  cout << "Test2"s << endl;
  FindAndPrint(test, 0); // элемента 0 нет в контейнере
  cout << "End of tests"s << endl;
}
