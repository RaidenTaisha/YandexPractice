#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

string ReadLine() {
  string s;
  getline(cin, s);
  return s;
}

int ReadLineWithNumber() {
  int result;
  cin >> result;
  ReadLine();
  return result;
}

int main() {
  const int queryCount = ReadLineWithNumber();

  vector<string> queries(queryCount);
  for (string& query : queries) {
    query = ReadLine();
  }
  const string buzzword = ReadLine();

  cout << count_if(queries.begin(), queries.end(), [buzzword](const string& query) {
    bool ret = false;
    size_t pos = query.find(buzzword);

    if (pos == string::npos) {
      ret = false;
    } else {
      bool first_subword = (pos == 0 || query[pos - 1] == ' ');
      bool secon_subword = (query.length() == pos + buzzword.length()
        || query[pos + buzzword.length()] == ' ');
      ret = first_subword && secon_subword;
    }
    return ret;
  });
  cout << endl;
}
