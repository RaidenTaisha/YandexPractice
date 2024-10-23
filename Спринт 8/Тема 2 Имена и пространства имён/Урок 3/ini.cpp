#include "ini.h"

#include <unordered_map>
#include <stdexcept>

using namespace std;
using namespace ini;

pair<string, string> ParseParam(string &line) {
  pair<string, string> result;
  auto devider_pos = line.find('=');
  auto param_begin = line.find_first_not_of(' ');
  auto param_end = line.find_last_not_of(' ', devider_pos - 1);
  auto value_begin = line.find_first_not_of(' ', devider_pos + 1);
  auto value_end = line.find_last_not_of(' ');

  if (!(devider_pos == string::npos
      || param_begin == string::npos
      || param_end == string::npos
      || value_begin == string::npos
      || value_end == string::npos
      || param_begin > param_end || value_begin > value_end)) {
    result.first = line.substr(param_begin, param_end - param_begin + 1);
    result.second = line.substr(value_begin, value_end - value_begin + 1);
  }

  return result;
}

bool IsSection(string &line) {
  return line.find('[') != string::npos && line.find(']') != string::npos;
}

void RemoveBraces(string &line) {
  line.erase(0, line.find('[') + 1);
  line.erase(line.find(']'), string::npos);
}

Section &Document::AddSection(const string &name) {
  return sections_[name];
}

Section const &Document::GetSection(const string &name) const {
  static Section empty;
  if (sections_.count(name) == 0 || sections_.at(name).empty()) {
    return empty;
  }

  return sections_.at(name);
}

size_t Document::GetSectionCount() const {
  return sections_.size();
}

Document ini::Load(istream &input) {
  Document document;
  string line;
  Section *current = nullptr;

  while (getline(input, line)) {
    if (line.find_first_not_of(' ', 0) == string::npos) {
      continue;
    }

    if (IsSection(line)) {
      RemoveBraces(line);
      current = &document.AddSection(line);
      continue;
    }

    // Параметры внутри секции
    if (current != nullptr) {
      auto pair = ParseParam(line);

      (*current)[pair.first] = pair.second;
    }
  }

  return document;
}
