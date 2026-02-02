#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <sstream>

FormulaError::FormulaError(Category category) : category_(category) {}

bool FormulaError::operator==(FormulaError rhs) const {
  return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
  using std::string_view_literals::operator""sv;

  switch(category_) {
    case FormulaError::Category::Ref:
      return "#REF!"sv;
    case FormulaError::Category::Value:
      return "#VALUE!"sv;
    case FormulaError::Category::Div0:
      return "#DIV/0!"sv;
    default:
      return {};
  }
}

std::ostream &operator<<(std::ostream &output, FormulaError fe) {
  return output << fe.ToString();
}

namespace {

class Formula : public FormulaInterface {
public:
  explicit Formula(const std::string &expression)
    : ast_{ParseFormulaAST(expression)} {}

  Value Evaluate(const SheetInterface &sheet) const override {
    auto lambda = [&sheet](Position pos) -> double {
      auto cell = sheet.GetCell(pos);
      auto value = cell ? cell->GetValue() : 0.0;

      if (std::holds_alternative<double>(value)) {
        return std::get<double>(value);
      } else if (std::holds_alternative<std::string>(value)) {
        std::string str_value = std::get<std::string>(value);

        if (str_value.empty()) {
          return 0;
        }

        char *endptr;
        errno = 0;
        auto res = std::strtod(str_value.c_str(), &endptr);

        if ((errno == ERANGE && res == HUGE_VAL)
            || (errno != 0 && res == 0)) {
          throw FormulaError(FormulaError::Category::Value);
        }

        // Цифры отсутствуют
        if (endptr == str_value.c_str()) {
          throw FormulaError(FormulaError::Category::Value);
        }

        // После числа есть ещё символы
        if (*endptr != '\0') {
          throw FormulaError(FormulaError::Category::Value);
        }

        return res;
      } else {
        throw std::get<FormulaError>(value);
      }
    };

    try {
      return ast_.Execute(lambda);
    } catch (FormulaError &fe) {
      return fe;
    }
  }

  std::string GetExpression() const override {
    std::stringstream ss;
    ast_.PrintFormula(ss);

    return ss.str();
  }

  std::vector<Position> GetReferencedCells() const override {
    std::vector<Position> result;
    Position prev = Position::NONE;

    for (auto &cell : ast_.GetCells()) {
      if (cell.IsValid() && !(cell == prev)) {
        result.push_back(cell);
        prev = cell;
      }
    }

    return result;
  }

private:
  FormulaAST ast_;
};

}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
  try {
    return std::make_unique<Formula>(std::move(expression));
  } catch (FormulaException &fe) {
    throw fe;
  }
}
