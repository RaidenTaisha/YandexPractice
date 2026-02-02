#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <iostream>
#include <optional>

void Sheet::SetCell(Position pos, std::string text) {
  ValidatePosition(pos);

  auto &cell_interface = cells_[pos];
  if (cell_interface == nullptr) {
    cell_interface = std::make_unique<Cell>(*this);
  }

  auto cell = dynamic_cast<Cell *>(cell_interface.get());
  cell->Set(std::move(text));
}

const CellInterface *Sheet::GetCell(Position pos) const {
  return const_cast<Sheet *>(this)->GetCell(pos);
}

CellInterface *Sheet::GetCell(Position pos) {
  ValidatePosition(pos);
  if (cells_.count(pos) == 0) {
    return nullptr;
  }

  return cells_[pos].get();
}

void Sheet::ClearCell(Position pos) {
  ValidatePosition(pos);

  if (cells_.count(pos) != 0) {
    auto cell = dynamic_cast<Cell *>(cells_[pos].get());
    cell->Clear();
    if (!cell->IsReferenced()) {
      cells_.erase(pos);
    }
  }
}

Size Sheet::GetPrintableSize() const {
  if (cells_.empty()) {
    return {};
  }
  Size result;

  for (const auto &[pos, ptr] : cells_) {
    if (ptr && !ptr->GetText().empty()) {
      result.rows = std::max(pos.row + 1, result.rows);
      result.cols = std::max(pos.col + 1, result.cols);
    }
  }

  return result;
}

void Sheet::PrintValues(std::ostream &output) const {
  auto lambda = [](const CellInterfacePtr &cell, std::ostream &output) {
    std::visit(
      [&output](auto &&arg){output << arg;},
      cell->GetValue());
  };

  PrintImpl(lambda, output);
}

void Sheet::PrintTexts(std::ostream &output) const {
  auto lambda = [](const CellInterfacePtr &cell, std::ostream &output) {
    output << cell->GetText();
  };

  PrintImpl(lambda, output);
}

void Sheet::ValidatePosition(Position pos) {
  using std::string_literals::operator""s;

  if (!pos.IsValid()) {
    throw InvalidPositionException{"InvalidPosition"s};
  }
}

std::unique_ptr<SheetInterface> CreateSheet() {
  return std::make_unique<Sheet>();
}
