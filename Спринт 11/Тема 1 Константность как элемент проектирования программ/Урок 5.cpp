#include <cassert>
#include <functional>
#include <string>

using namespace std;

template <typename T>
class LazyValue {
 public:
  explicit LazyValue(std::function<T()> init) : init_(init) {}

  bool HasValue() const {
    return nullptr != value_;
  }

  const T& Get() const {
    if (nullptr == value_) {
      static T __{init_()};
      value_ = &__;
    }
    return *value_;
  }

 private:
  mutable T *value_ = nullptr;
  std::function<T()> init_;
};

void UseExample() {
  const string big_string = "Giant amounts of memory"s;

  LazyValue<string> lazy_string([&big_string] {
    return big_string;
  });

  assert(!lazy_string.HasValue());
  assert(lazy_string.Get() == big_string);
  assert(lazy_string.Get() == big_string);
}

void TestInitializerIsntCalled() {
  bool called = false;

  {
    LazyValue<int> lazy_int([&called] {
      called = true;
      return 0;
    });
  }
  assert(!called);
}

int main() {
  UseExample();
  TestInitializerIsntCalled();
}
