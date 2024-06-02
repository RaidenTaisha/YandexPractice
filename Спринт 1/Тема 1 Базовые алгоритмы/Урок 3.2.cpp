#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

bool StartsWithA(const string& buffer) {
  return (!buffer.empty() && (buffer[0] == 'a' || buffer[0] == 'A'));
}

int CountStartsWithA(const vector<string>& buffer) {
  return count_if(buffer.begin(), buffer.end(), StartsWithA);
}

int main() {
  CountStartsWithA({"Anna"s, "Banny"s, "Clara"s, "all others"s});
  return 0;
}
