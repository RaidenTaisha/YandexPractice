#include <cassert>
#include <string>
#include <string_view>

using namespace std::literals;

// Решение, использующее fold expression
template <typename T0, typename... Ts>
bool EqualsToOneOf(const T0& v0, const Ts&... vs) {
  return (... || (v0 == vs));
}

bool IsExpectedAnswer(std::string_view answer) {
  return EqualsToOneOf(answer, "yes"sv, "no"sv, "don't know"sv);
}

int main() {
  assert(EqualsToOneOf("hello"sv, "hi"s, "hello"s));
  assert(!EqualsToOneOf(1, 10, 2, 3, 6));
  assert(!EqualsToOneOf(8));
}
