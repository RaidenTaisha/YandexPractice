#include <iostream>
#include <string>

using namespace std;

// Определяет, будет ли слово палиндромом
// text может быть строкой, содержащей строчные буквы английского алфавита и пробелы
// Пустые строки и строки, состоящие только из пробелов, - это не палиндромы
bool IsPalindrome(const string &text) {
  bool ret = true;
  string buf;
  for (const auto letter : text) {
    if (!isspace(letter)) {
      buf.push_back(letter);
    }
  }
  if (buf.empty()) {
    ret = false;
  } else {
    for (size_t i = 0; i < buf.size() / 2; i++) {
      if (buf[i] != buf[buf.size() - 1 - i]) {
        ret = false;
        break;
      }
    }
  }
  return ret;
}

int main() {
  string text;
  getline(cin, text);

  if (IsPalindrome(text)) {
    cout << "palindrome"s << endl;
  } else {
    cout << "not a palindrome"s << endl;
  }
}