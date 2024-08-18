#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using filesystem::path;

path operator ""_p(const char *data, std::size_t sz) {
  return path(data, data + sz);
}

void PrintTree(ostream& dst, const path& p, int offset) {
  error_code err;
  vector<pair<string, filesystem::file_status>> paths;

  for (const auto& elem : filesystem::directory_iterator(p)) {
    auto status = filesystem::status(elem, err);
    string path_s = elem.path().filename().string();
    paths.push_back({path_s, status});
  }

  sort(paths.begin(),
       paths.end(),
       [](const pair<string, filesystem::file_status>& p1,
          const pair<string, filesystem::file_status>& p2) {
         return p1.first > p2.first && p1.second.type() == p2.second.type();
       });

  for (const auto& [path, status] : paths) {
    if (status.type() == filesystem::file_type::directory) {
      dst << string(offset, ' ') << path << endl;
      PrintTree(dst, p / path, offset + 2);
    } else {
      dst << string(offset, ' ') << path << endl;
    }
  }
}

// напишите эту функцию
void PrintTree(ostream& dst, const path& p) {
  dst << p.string() << endl;
  PrintTree(dst, p, 2);
}

int main() {
  error_code err;
  filesystem::remove_all("test_dir", err);
  filesystem::create_directories("test_dir"_p / "a"_p, err);
  filesystem::create_directories("test_dir"_p / "b"_p, err);

  ofstream("test_dir"_p / "b"_p / "f1.txt"_p);
  ofstream("test_dir"_p / "b"_p / "f2.txt"_p);
  ofstream("test_dir"_p / "a"_p / "f3.txt"_p);

  ostringstream out;
  PrintTree(out, "test_dir"_p);
  assert(out.str() ==
      "test_dir\n"
      "  b\n"
      "    f2.txt\n"
      "    f1.txt\n"
      "  a\n"
      "    f3.txt\n"s
  );
}
