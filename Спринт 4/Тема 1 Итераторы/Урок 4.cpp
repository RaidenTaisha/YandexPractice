#include <algorithm>
#include <iostream>
#include <set>
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

template <class Iterator>
auto MakeSet(Iterator range_begin, Iterator range_end) {
  return set(range_begin, range_end);
}

template <class Iterator>
auto MakeVector(Iterator range_begin, Iterator range_end) {
  return vector(range_begin, range_end);
}

int main() {
  vector<string> langs = {"Python"s, "Java"s, "C#"s, "Ruby"s, "C++"s, "C++"s, "C++"s, "Ruby"s, "Java"s};
  auto unique_langs = MakeVector(langs.begin(), langs.end());
  PrintRange(unique_langs.begin(), unique_langs.end());
}
