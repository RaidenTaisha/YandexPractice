#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

using namespace std;

string GetLine(istream& in) {
  string s;
  getline(in, s);
  return s;
}

void CreateFiles(const string& config_file) {
  ifstream ist(config_file);
  string filename;
  while (!ist.eof()) {
    ofstream ost;
    string line = GetLine(ist);
    if (!line.empty()) {
      if (line.front() != '>') {
        filename = line;
      } else {
        ost.open(filename, std::ios_base::app);
        ost << line.substr(1, line.size() - 1) << '\n';
        ost.close();
      }
    }
  }
}


int main() {
  ofstream("test_config.txt"s) << "a.txt\n"
                                  ">10\n"
                                  ">abc\n"
                                  "b.txt\n"
                                  ">123"s;

  CreateFiles("test_config.txt"s);
  ifstream in_a("a.txt"s);
  assert(GetLine(in_a) == "10"s && GetLine(in_a) == "abc"s);

  ifstream in_b("b.txt"s);
  assert(GetLine(in_b) == "123"s);
}
