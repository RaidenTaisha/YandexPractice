#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

size_t GetFileSize(const string& file) {
  ifstream ist(file);
  ist.seekg(0, std::ifstream::end);
  size_t length = ist.tellg();
  ist.seekg(0, std::ifstream::beg);

  if (length == 0) {
    return string::npos;
  }
  return length;
}

int main() {
  ofstream("test.txt") << "123456789"s;
  assert(GetFileSize("test.txt"s) == 9);

  ofstream test2("test2.txt"s);
  test2.seekp(1000);
  test2 << "abc"s;
  test2.flush();

  assert(GetFileSize("test2.txt"s) == 1003);
  assert(GetFileSize("missing file name"s) == string::npos);
}
