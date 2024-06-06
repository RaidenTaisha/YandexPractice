#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

string ToLowerCase(string s) {
  for (char& ch : s) {
    ch = tolower(static_cast<unsigned char>(ch));
  }
  return s;
}

int main() {
  unsigned count_words = 0;
  vector<string> words_list;

  cin >> count_words;
  for (unsigned i = 0; i < count_words; i++) {
    string word;
    cin >> word;
    words_list.emplace_back(word);
  }

  sort(words_list.begin(), words_list.end(),
    [](const string& lhs, const string& rhs) {
      return ToLowerCase(lhs) < ToLowerCase(rhs);
    });

  for (auto & word : words_list) {
    cout << word << " ";
  }
  cout << endl;
}
