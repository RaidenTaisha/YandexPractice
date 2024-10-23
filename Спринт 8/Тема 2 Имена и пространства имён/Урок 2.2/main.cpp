#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <vector>

// не меняйте файлы xml.h и xml.cpp
#include "xml.h"

using namespace std;

struct Spending {
  string category;
  int amount;
};

int CalculateTotalSpendings(const vector<Spending>& spendings) {
  return accumulate(spendings.begin(), spendings.end(), 0,
    [](int current, const Spending& spending) {
      return current + spending.amount;
    });
}

string FindMostExpensiveCategory(const vector<Spending>& spendings) {
  assert(!spendings.empty());
  auto compare_by_amount = [](const Spending& lhs, const Spending& rhs) {
    return lhs.amount < rhs.amount;
  };
  return max_element(begin(spendings), end(spendings), compare_by_amount)->category;
}

vector<Spending> LoadFromXml(istream& input) {
  std::vector<Spending> result;

  auto document = Load(input);
  auto &root = document.GetRoot();
  auto &children = root.Children();

  for (const auto &child : children) {
    auto category = child.AttributeValue<std::string>("category");
    auto amount = child.AttributeValue<int>("amount");

    result.push_back({category, amount});
  }

  return result;
}

int main() {
  const vector<Spending> spendings = LoadFromXml(cin);
  cout << "Total "sv << CalculateTotalSpendings(spendings) << '\n';
  cout << "Most expensive is "sv << FindMostExpensiveCategory(spendings) << '\n';
}
