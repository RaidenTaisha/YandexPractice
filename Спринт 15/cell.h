#pragma once

#include "common.h"
#include "formula.h"

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stack>

class Sheet;

class Cell : public CellInterface {
public:
  using Positions = std::vector<Position>;

  explicit Cell(Sheet &sheet);
  ~Cell() override = default;

  void Set(std::string text);
  void Clear();

  Value GetValue() const override;
  std::string GetText() const override;

  Positions GetReferencedCells() const override;
  bool IsReferenced() const;

private:
  class Impl;
  class EmptyImpl;
  class TextImpl;
  class FormulaImpl;

  void SetImpl(std::string text);
  void CheckForCircularDependencies(const Positions &referenced_cells) const;
  CellInterface *CreateEmptyCell(const Position &pos) const;

  static void AddToStack(std::stack<Position> &destination, const Positions &source);
  static std::stack<Position> CreateStack(const Positions &referenced_cells);

  void SetParents();

  std::unique_ptr<Impl> impl_;
  Sheet &sheet_;
  std::set<Cell *> parents_;   // Ячейки, от которых зависит текущая
  std::set<Cell *> children_;  // Ячейки, которые зависят от текущей
};

class Cell::Impl {
public:
  virtual ~Impl() = default;

  virtual Value GetValue(const Sheet &sheet) const = 0;
  virtual std::string GetText() const = 0;
  virtual Positions GetReferencedCells() const;

  virtual void ClearCache() {};

private:
  Positions empty_vector_;
};

class Cell::EmptyImpl : public Cell::Impl {
public:
  Value GetValue(const Sheet &) const override;
  std::string GetText() const override;

private:
  std::string empty_text_;
};

class Cell::TextImpl : public Cell::Impl {
public:
  explicit TextImpl(std::string text);

  Value GetValue(const Sheet &) const override;
  std::string GetText() const override;

private:
  std::string text_;
};

class Cell::FormulaImpl : public Cell::Impl {
public:
  explicit FormulaImpl(const std::string &text);

  Value GetValue(const Sheet &sheet) const override;
  std::string GetText() const override;
  Positions GetReferencedCells() const override;
  void ClearCache() override;

private:
  std::unique_ptr<FormulaInterface> formula_;
  std::optional<FormulaInterface::Value> cache_;
};
