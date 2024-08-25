#include <cassert>
#include <map>
#include <string>
#include <string_view>

using namespace std;

class Translator {
 public:
  void Add(string_view source, string_view target) {
    for (auto &entry : dict_forward_) {
      if (entry.first == source) {
        dict_backward_[entry.second] = string(source);
        dict_forward_[entry.first] = string(target);
      }
    }
    dict_forward_.insert({string(source), string(target)});
    dict_backward_.insert({ string(target), string(source) });
  }
  string TranslateForward(string_view source) const {
    for (auto &entry : dict_forward_) {
      if (entry.first == source) {
        return entry.second;
      }
    }
    return ""s;
  }
  string TranslateBackward(string_view target) const {
    for (auto &entry : dict_backward_) {
      if (entry.first == target) {
        return entry.second;
      }
    }
    return ""s;
  }

 private:
  map<string, string> dict_forward_;
  map<string, string> dict_backward_;
};

void TestSimple() {
  Translator translator;
  translator.Add(string("okno"s), string("window"s));
  translator.Add(string("stol"s), string("table"s));

  assert(translator.TranslateForward("okno"s) == "window"s);
  assert(translator.TranslateBackward("table"s) == "stol"s);
  assert(translator.TranslateForward("table"s) == ""s);
}

int main() {
  TestSimple();
}