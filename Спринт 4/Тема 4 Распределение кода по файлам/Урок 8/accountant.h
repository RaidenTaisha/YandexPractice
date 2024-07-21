#pragma once
#include <utility>

#include "square_calculation.h"
#include "wall.h"

class Accountant {
public:
    template <class BuildingObject>
    double CalcPaintNeeded(const BuildingObject& building_object) const;
    template <class BuildingObject>
    double CalcBricksNeeded(const BuildingObject& building_object) const;
};

template <class BuildingObject>
double Accountant::CalcPaintNeeded(const BuildingObject& building_object) const {
  std::pair<double, double> sizes = building_object.GetSizes();
  return CalcSquare(sizes.first, sizes.second) * 0.4;
}

template <class BuildingObject>
double Accountant::CalcBricksNeeded(const BuildingObject& building_object) const {
  std::pair<double, double> sizes = building_object.GetSizes();
  return CalcSquare(sizes.first, sizes.second) * 5;
}