#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using filesystem::path;

enum TypeIncludeFind {
  NOT_FOUND,
  LOCAL_FOUND,
  GLOBAL_FOUND
};

path operator""_p(const char* data, std::size_t sz) {
  return path(data, data + sz);
}

std::optional<path> FileExistInIncludeDirectories(const path& filename, const vector<path>& include_directories) {
  for (const auto& include_directory : include_directories) {
    if (filesystem::exists(include_directory / filename)) {
      return include_directory;
    }
  }

  return nullopt;
}

bool PreprocessRecursive(stringstream& isst, ostringstream& osst, const path& file_path, const vector<path>& include_directories) {
  bool result = true;
  const regex local_include_reg(R"(\s*#\s*include\s*\"([^"]*)\"\s*)");
  const regex global_include_reg(R"(\s*#\s*include\s*<([^>]*)>\s*)");
  smatch include_match;

  const path& dir = file_path.parent_path();

  string line;
  uint32_t line_count = 0;
  while (std::getline(isst, line)) {
    string include_file;
    ++line_count;
    int type_found_include = NOT_FOUND;
    if (regex_match(line, include_match, local_include_reg)) {
      include_file = include_match[1];
      type_found_include = LOCAL_FOUND;
    } else if (regex_match(line, include_match, global_include_reg)) {
      include_file = include_match[1];
      type_found_include = GLOBAL_FOUND;
    }

    if (type_found_include == NOT_FOUND) {
      osst << line << endl;
    } else {
      path include_path;
      if (type_found_include == LOCAL_FOUND && filesystem::exists(dir / include_file)) {
        include_path = dir / include_file;
      } else if (auto find_dir = FileExistInIncludeDirectories(include_file, include_directories)) {
        include_path = find_dir.value() / include_file;
      } else {
        cout << "unknown include file "s + include_file +
            " at file "s + file_path.string() + " at line "s + to_string(line_count) << endl;
        return false;
      }

      ifstream ist(include_path);
      stringstream isst_new;
      isst_new << ist.rdbuf();

      result = PreprocessRecursive(isst_new, osst, include_path, include_directories);
    }
  }

  return result;
}

// напишите эту функцию
bool Preprocess(const path& in_file, const path& out_file, const vector<path>& include_directories) {
  ifstream ist(in_file);
  if (not ist.is_open()) {
    return false;
  }

  stringstream isst;
  isst << ist.rdbuf();

  ostringstream osst;
  bool result = PreprocessRecursive(isst, osst, in_file, include_directories);

  ofstream ost(out_file);
  ost << osst.str();

  return result;
}

string GetFileContents(string file) {
  ifstream stream(file);

  // конструируем string по двум итераторам
  return {(istreambuf_iterator<char>(stream)), istreambuf_iterator<char>()};
}

void Test() {
  error_code err;
  filesystem::remove_all("sources"_p, err);
  filesystem::create_directories("sources"_p / "include2"_p / "lib"_p, err);
  filesystem::create_directories("sources"_p / "include1"_p, err);
  filesystem::create_directories("sources"_p / "dir1"_p / "subdir"_p, err);

  {
    ofstream file("sources/a.cpp");
    file << "// this comment before include\n"
            "#include \"dir1/b.h\"\n"
            "// text between b.h and c.h\n"
            "#include \"dir1/d.h\"\n"
            "\n"
            "int SayHello() {\n"
            "    cout << \"hello, world!\" << endl;\n"
            "#   include<dummy.txt>\n"
            "}\n"s;
  }
  {
    ofstream file("sources/dir1/b.h");
    file << "// text from b.h before include\n"
            "#include \"subdir/c.h\"\n"
            "// text from b.h after include"s;
  }
  {
    ofstream file("sources/dir1/subdir/c.h");
    file << "// text from c.h before include\n"
            "#include <std1.h>\n"
            "// text from c.h after include\n"s;
  }
  {
    ofstream file("sources/dir1/d.h");
    file << "// text from d.h before include\n"
            "#include \"lib/std2.h\"\n"
            "// text from d.h after include\n"s;
  }
  {
    ofstream file("sources/include1/std1.h");
    file << "// std1\n"s;
  }
  {
    ofstream file("sources/include2/lib/std2.h");
    file << "// std2\n"s;
  }

  assert((!Preprocess("sources"_p / "a.cpp"_p, "sources"_p / "a.in"_p,
                      {"sources"_p / "include1"_p,"sources"_p / "include2"_p})));

  ostringstream test_out;
  test_out << "// this comment before include\n"
              "// text from b.h before include\n"
              "// text from c.h before include\n"
              "// std1\n"
              "// text from c.h after include\n"
              "// text from b.h after include\n"
              "// text between b.h and c.h\n"
              "// text from d.h before include\n"
              "// std2\n"
              "// text from d.h after include\n"
              "\n"
              "int SayHello() {\n"
              "    cout << \"hello, world!\" << endl;\n"s;

  assert(GetFileContents("sources/a.in"s) == test_out.str());
}

int main() {
  Test();
}
