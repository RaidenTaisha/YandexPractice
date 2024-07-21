#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

template <class Iterator>
void PrintRange(Iterator range_begin, Iterator range_end) {
  for (auto it = range_begin; it != range_end; ++it) {
    cout << *it << " "s;
  }
  cout << endl;
}

template <class Container>
void EraseAndPrint(Container& container, int position)  {
  auto it = container.begin();
  for (int i = 0; i < position; i++)
    it++;
  PrintRange(container.begin(), it);
  auto removed_element_position = container.erase(it);
  PrintRange(removed_element_position, container.end());
}

template <class Container>
void EraseAndPrint(Container& container, int position, int left, int right)  {
  container.erase(container.begin() + position);
  PrintRange(container.begin(), container.end());
  container.erase(container.begin() + left, container.begin() + right);
  PrintRange(container.begin(), container.end());
}

int main() {
  vector<string> langs = {"Python"s, "Java"s, "C#"s, "Ruby"s, "C++"s};
  EraseAndPrint(langs, 0, 0, 2);
  return 0;
}
