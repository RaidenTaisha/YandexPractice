#include "cell.h"
#include "common.h"
#include "sheet.h"

#include <optional>
#include <string>


Cell::Cell(Sheet &sheet) : impl_(std::make_unique<EmptyImpl>()), sheet_(sheet) {}

void Cell::Set(std::string text) {
  if (text == impl_->GetText()) {
    return;
  }

  SetImpl(std::move(text));
  for (auto parent : parents_)
    parent->children_.erase(this);
  parents_.clear();
  SetParents();
  impl_->ClearCache();
  for (auto child : children_)
    child->Clear();
}

void Cell::Clear() {
  impl_ = std::make_unique<EmptyImpl>();
  for (auto parent : parents_)
    parent->children_.erase(this);
  parents_.clear();
  impl_->ClearCache();
  for (auto child : children_)
    child->Clear();
}

Cell::Value Cell::GetValue() const {
  return impl_->GetValue(sheet_);
}

std::string Cell::GetText() const {
  return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
  return impl_->GetReferencedCells();
}

void Cell::SetParents() {
  for (const auto &parent_pos : GetReferencedCells()) {
    auto *parent = sheet_.GetCell(parent_pos);
    if (parent == nullptr) {
      parent = CreateEmptyCell(parent_pos);
    }

    static_cast<Cell *>(parent)->children_.insert(this);
    parents_.insert(static_cast<Cell *>(parent));
  }
}

CellInterface *Cell::CreateEmptyCell(const Position &pos) const {
  sheet_.SetCell(pos, "");
  return sheet_.GetCell(pos);
}

void
Cell::CheckForCircularDependencies(const Positions &referenced_cells) const {
  std::stack<Position> stack_positions = CreateStack(referenced_cells);
  std::map<Position, bool> visited_cells;

  while (!stack_positions.empty()) {
    Position current_pos = stack_positions.top();
    stack_positions.pop();
    bool check_pos = visited_cells[current_pos];

    if (check_pos) {
      continue;
    }

    visited_cells[current_pos] = true;
    const CellInterface *current_cell = sheet_.GetCell(current_pos);

    if (current_cell == this) {
      throw CircularDependencyException{"Circular dependency"};
    }

    if (current_cell == nullptr) {
      current_cell = CreateEmptyCell(current_pos);
    }

    AddToStack(stack_positions, current_cell->GetReferencedCells());
  }
}

void Cell::SetImpl(std::string text) {
  if (text.empty()) {
    impl_ = std::make_unique<EmptyImpl>();
  } else if (text.size() > 1 && text.front() == FORMULA_SIGN) {
    std::unique_ptr<FormulaImpl> temp_impl;

    try {
      temp_impl = std::make_unique<FormulaImpl>(text.substr(1));
    } catch (std::exception &e) {
      throw FormulaException("Formula error");
    }

    auto referenced_cells = temp_impl->GetReferencedCells();
    CheckForCircularDependencies(referenced_cells);

    impl_ = std::move(temp_impl);
  } else {
    impl_ = std::make_unique<TextImpl>(std::move(text));
  }
}

void
Cell::AddToStack(std::stack<Position> &destination, const Positions &source) {
  for (Position pos : source) {
    destination.push(pos);
  }
}

std::stack<Position> Cell::CreateStack(const Positions &referenced_cells) {
  std::stack<Position> stack_positions;
  AddToStack(stack_positions, referenced_cells);

  return stack_positions;
}

Cell::Positions Cell::Impl::GetReferencedCells() const {
  return empty_vector_;
}

bool Cell::IsReferenced() const {
  return !children_.empty();
}

CellInterface::Value Cell::EmptyImpl::GetValue(const Sheet &) const {
  return empty_text_;
}

std::string Cell::EmptyImpl::GetText() const {
  return empty_text_;
}

Cell::TextImpl::TextImpl(std::string text)
  : text_(std::move(text)) {
}

CellInterface::Value Cell::TextImpl::GetValue(const Sheet &) const {
  if (!text_.empty() && text_.front() == ESCAPE_SIGN) {
    return text_.substr(1);
  } else {
    return text_;
  }
}

std::string Cell::TextImpl::GetText() const {
  return text_;
}

Cell::FormulaImpl::FormulaImpl(const std::string &text)
  : formula_(ParseFormula(text)) {
}

CellInterface::Value Cell::FormulaImpl::GetValue(const Sheet &sheet) const {
  auto formula_evaluate = formula_->Evaluate(sheet);

  if (std::holds_alternative<double>(formula_evaluate)) {
    return std::get<double>(formula_evaluate);
  } else {
    return std::get<FormulaError>(formula_evaluate);
  }
}

std::string Cell::FormulaImpl::GetText() const {
  return '=' + formula_->GetExpression();
}

Cell::Positions Cell::FormulaImpl::GetReferencedCells() const {
  return formula_->GetReferencedCells();
}

void Cell::FormulaImpl::ClearCache() {
  cache_.reset();
}
