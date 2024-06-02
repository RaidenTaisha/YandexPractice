#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main() {
  int count;
  cin >> count;
  vector<pair<int, string>> participants;
  for (int i = 0; i < count; ++i) {
    string name;
    int age;
    cin >> name >> age;
    participants.push_back({age, name});
  }
  sort(participants.begin(), participants.end());
  reverse(participants.begin(), participants.end());
  for (const auto& participant : participants) {
    cout << participant.second  << endl;
  }
}
