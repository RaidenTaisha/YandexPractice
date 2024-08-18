#include "log_duration.h"

#include <iostream>

using namespace std;

class StreamUntier {
 public:
  StreamUntier(std::istream & stream) : stream_(stream)
  {};

  ~StreamUntier()
  {
    stream_.tie(tied_before_);
  }

 private:
  istream& stream_;
  ostream* tied_before_ = stream_.tie(nullptr);
};

int main() {
  LOG_DURATION("\\n with tie"s);

  StreamUntier guard(cin);
  int i;
  while (cin >> i) {
    cout << i * i << "\n"s;
  }

  return 0;
}
