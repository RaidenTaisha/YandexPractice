#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

using namespace std;

template<typename Hash>
int FindCollisions(const Hash &hasher, istream &text) {
  int res = 0;
  unordered_map<size_t, unordered_set<string>> words;
  string word;
  while (text >> word) {
    words[hasher(word)].insert(std::move(word));
  }
  for (auto &it : words) {
    res += static_cast<int>((it.second.size()) - 1);
  }
  return res;
}

// Это плохой хешер. Его можно использовать для тестирования.
// Подумайте, в чём его недостаток
struct HasherDummy {
  size_t operator()(const string &str) const {
    size_t res = 0;
    for (char c : str) {
      res += static_cast<size_t>(c);
    }
    return res;
  }
};

#include <sstream>
struct DummyHash {
  size_t operator()(const string &) const {
    return 42;
  }
};

int main() {
  DummyHash dummy_hash;
  {
    istringstream stream("a a a a"s);
    cout << FindCollisions(dummy_hash, stream) << endl;
  }
  {
    istringstream stream("a a a b b"s);
    cout << FindCollisions(dummy_hash, stream) << endl;
  }
}