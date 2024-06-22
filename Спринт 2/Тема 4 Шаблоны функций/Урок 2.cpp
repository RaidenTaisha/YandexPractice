#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

map<string, int> ComputeTermFreqs(const vector<string> &words) {
  map<string, int> ret;
  for (const auto &word : words) {
    ret[word]++;
  }
  return ret;
}

int main() {
  const vector<string> words = {"first"s, "time"s, "first"s, "class"s};
  for (const auto &[word, freq] : ComputeTermFreqs(words)) {
    cout << word << " x "s << freq << endl;
  }
  return 0;
}
