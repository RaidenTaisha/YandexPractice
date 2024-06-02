#include <set>
#include <string>
#include <numeric>

using namespace std;

string Adding(string a, string b) {
  string ret = a;
  if (!b.empty()) {
    ret = a + " -" + b;
  }
  return ret;
}

string AddStopWords(const string& query, const set<string>& stop_words) {
  return accumulate(stop_words.begin(), stop_words.end(), query, Adding);
}
