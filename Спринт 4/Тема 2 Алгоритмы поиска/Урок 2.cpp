#include <iostream>
#include <string>

using namespace std;

void PrintSpacesPositions([[maybe_unused]] std::string& str) {
  if(!str.empty()) {
    auto next_pos = str.find(' ');
    do {
      std::cout << next_pos << std::endl;
      next_pos = str.find(' ', next_pos + 1);
    } while (next_pos != std::string::npos);
  }
}

int main() {
  std::string str = "He said: one and one and one is three";
  PrintSpacesPositions(str);
  return 0;
}
