#include "common.h"

#include <cctype>
#include <algorithm>
#include <sstream>
#include <tuple>

namespace {
int
power(int num, int deg) {
  int result = 1;
  for (long i = 0; i < deg; i++) {
    result *= num;
  }

  return result;
}
}

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

const Position Position::NONE = {-1, -1};

bool Position::operator==(const Position rhs) const {
  return row == rhs.row && col == rhs.col;
}

bool Position::operator<(const Position rhs) const {
  return std::tie(row, col) < std::tie(rhs.row, rhs.col);
}

bool Position::IsValid() const {
  return row >= 0 && col >= 0 && row < MAX_ROWS && col < MAX_COLS;
}

std::string Position::ToString() const {
  if (!IsValid())
    return {};

  std::string result;
  result.reserve(MAX_POSITION_LENGTH);
  int c = col;

  while (c >= 0) {
    result.insert(result.begin(), 'A' + static_cast<char>(c % LETTERS));
    c /= LETTERS;
    c -= 1;
  }

  result += std::to_string(row + 1);
  return result;
}

Position Position::FromString(std::string_view str) {
  auto letters_end = std::find_if(str.begin(), str.end(),
                                  [](const auto c) {
                                    return !std::isalpha(c) || !std::isupper(c);
                                  });
  auto digits_end = std::find_if(letters_end, str.end(),
                                 [](const auto c) {
                                   return !std::isdigit(c);
                                 });

  std::string letters(str.begin(), letters_end);
  std::string digits(std::next(str.begin(), letters.size()), digits_end);

  if (letters.empty() || digits.empty() || letters.size() > MAX_POS_LETTER_COUNT || digits_end != str.end())
    return Position::NONE;

  int letters_count = 0;
  int digits_count = 0;
  int row = 0;
  int col = 0;
  for (auto rbegin = digits.rbegin(), rend = digits.rend();
       rbegin != rend; ++rbegin) {
    const auto ch = *rbegin;
    row += (ch - '0') * power(10, digits_count);
    digits_count++;
  }
  for (auto rbegin = letters.rbegin(), rend = letters.rend();
       rbegin != rend; ++rbegin) {
    const auto ch = *rbegin;
    col += (ch - 'A' + 1) * power(26, letters_count);
    letters_count++;
  }

  return {row - 1, col - 1};
}

bool Size::operator==(Size rhs) const {
  return cols == rhs.cols && rows == rhs.rows;
}