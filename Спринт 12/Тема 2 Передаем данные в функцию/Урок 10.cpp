#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <type_traits>

class Domain {
 public:
  explicit Domain(const std::string &domain)
      : domain_(domain.rbegin(), domain.rend()) {
    domain_.push_back('.');
  }

  [[nodiscard]] bool IsSubdomain(const Domain &other) const {
    return domain_.size() >= other.domain_.size()
        && std::equal(other.domain_.begin(),
                      other.domain_.end(),
                      domain_.begin());
  }

  [[nodiscard]] const std::string &GetDomain() const {
    return domain_;
  }

 private:
  std::string domain_;
};

class DomainChecker {
 public:
  template<typename It>
  DomainChecker(It begin, It end) : forbidden_(begin, end) {
    std::sort(forbidden_.begin(), forbidden_.end(),
              [](const auto &lhs, const auto &rhs) {
                return lhs.GetDomain() < rhs.GetDomain();
              });

    auto pos_to_erase = std::unique(forbidden_.begin(), forbidden_.end(),
                                    [](const auto &lhs, const auto &rhs) {
                                      return rhs.IsSubdomain(lhs);
                                    });
    forbidden_.erase(pos_to_erase, forbidden_.end());
  }

  [[nodiscard]] const std::vector<Domain> &GetForbidden() const {
    return forbidden_;
  }

  bool IsForbidden(const Domain &domain) {
    auto pos = std::upper_bound(forbidden_.begin(), forbidden_.end(), domain,
                                [](const auto &lhs, const auto &rhs) {
                                  return lhs.GetDomain() < rhs.GetDomain();
                                });

    return !(pos == forbidden_.begin()) && domain.IsSubdomain(*std::prev(pos));
  }

 private:
  std::vector<Domain> forbidden_;
};

// Можно считать только целое число доменов,
// для этого выключаем все не удовлетворяющие условию std::is_integral_v типы
template<typename Number, typename = std::enable_if_t<std::is_integral_v<Number>>>
Number ReadNumberOnLine(std::istream &input) {
  std::string line;
  std::getline(input, line);

  Number num;
  std::istringstream(line) >> num;

  return num;
}

// Можно считать только целое число доменов,
// для этого выключаем все не удовлетворяющие условию std::is_integral_v типы
template<typename Number, typename = std::enable_if_t<std::is_integral_v<Number>>>
std::vector<Domain> ReadDomains(std::istream &input, Number num) {
  std::vector<Domain> result;

  while (num > 0) {
    std::string line;
    std::getline(input, line);
    result.emplace_back(line);
    --num;
  }

  return result;
}

int main() {
  const std::vector<Domain> forbidden_domains
      = ReadDomains(std::cin, ReadNumberOnLine<size_t>(std::cin));

  DomainChecker checker(forbidden_domains.begin(), forbidden_domains.end());

  const std::vector<Domain> test_domains
      = ReadDomains(std::cin, ReadNumberOnLine<size_t>(std::cin));

  for (const Domain &domain : test_domains) {
    std::cout << (checker.IsForbidden(domain)
        ? "Bad" : "Good") << std::endl;
  }
}
