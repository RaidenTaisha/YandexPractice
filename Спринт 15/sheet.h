#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <iostream>
#include <map>

class Sheet : public SheetInterface {
public:
  using CellInterfacePtr = std::unique_ptr<CellInterface>;
  using Table = std::map<Position, CellInterfacePtr>;

  ~Sheet() override = default;

  void SetCell(Position pos, std::string text) override;

  const CellInterface *GetCell(Position pos) const override;
  CellInterface *GetCell(Position pos) override;

  void ClearCell(Position pos) override;

  Size GetPrintableSize() const override;

  void PrintValues(std::ostream &output) const override;
  void PrintTexts(std::ostream &output) const override;

private:
  static void ValidatePosition(Position pos) ;

  template<typename PrintContext>
  void PrintImpl(PrintContext print_context, std::ostream &output) const;

  Table cells_;
};

template<typename PrintContext>
void Sheet::PrintImpl(PrintContext print_context, std::ostream &output) const {
  auto printable_size = GetPrintableSize();
  auto printable_rows = printable_size.rows;
  auto printable_cols = printable_size.cols;

  for (int row = 0; row < printable_rows; ++row) {
    for (int col = 0; col < printable_cols; ++col) {
      if (cells_.count({row, col}) != 0) {
        print_context(cells_.at({row, col}), output);
      }

      if (col < printable_cols - 1) {
        output << '\t';
      }
    }
    output << '\n';
  }
}
