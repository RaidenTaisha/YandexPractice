#pragma once

#include "entities.h"
#include "summing_segment_tree.h"

#include <cmath>

struct BulkMoneyAdder {
  double delta = {};
};

struct BulkTaxApplier {
  double multiplier = 1;
};

struct BulkMoneySpender {
  double delta {};
};

class BulkLinearUpdater {
 public:
  BulkLinearUpdater() = default;

  BulkLinearUpdater(const BulkMoneyAdder& add)
      : add_(add) {
  }

  BulkLinearUpdater(const BulkTaxApplier& tax)
      : tax_(tax) {
  }

  BulkLinearUpdater(const BulkMoneySpender& spend)
      : spend_(spend) {
  }

  void CombineWith(const BulkLinearUpdater& other) {
    tax_.multiplier *= other.tax_.multiplier;
    add_.delta = add_.delta * other.tax_.multiplier + other.add_.delta;
    spend_.delta = spend_.delta + other.spend_.delta;
  }

  DayBudget Collapse(DayBudget origin, IndexSegment segment) const {
    const auto income = origin.income * tax_.multiplier + add_.delta * static_cast<double>(segment.length());
    const auto spend = origin.spend + spend_.delta * static_cast<double>(segment.length());
    return {income, spend};
  }

 private:
  // Apply tax first, then add.
  BulkTaxApplier tax_;
  BulkMoneyAdder add_;
  BulkMoneySpender spend_;
};
