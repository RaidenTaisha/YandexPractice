#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <string_view>

using namespace std::string_view_literals;
// Вместо "сырых" массивов фиксированного размера лучше использовать std::array.
// Тогда можно будет использовать метод size() вместо хранения константы, задающей размер массива
const std::array PLANETS {
    "Mercury"sv, "Venus"sv, "Earth"sv,
    "Mars"sv, "Jupiter"sv, "Saturn"sv,
    "Uranus"sv, "Neptune"sv,
};

bool IsPlanet(std::string_view name) {
  // Следует использовать подходящие стандартные алгоритмы, а не писать циклы вручную.
  // Тогда будет меньше вероятность совершить ошибку
  return find(PLANETS.begin(), PLANETS.end(), name) != PLANETS.end();
}

void Test(std::string_view name) {
  std::cout << name << " is " << (IsPlanet(name) ? ""sv : "NOT "sv) << "a planet"sv << std::endl;
}

int main() {
  std::string name;
  getline(std::cin, name);
  Test(name);
}