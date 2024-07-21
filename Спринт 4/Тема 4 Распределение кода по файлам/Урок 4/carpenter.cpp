#include "square_calculation.h"
#include "carpenter.h"

int Carpenter::CalcShelves(const Wall &wall) {
  return CalcSquare(wall.GetHeight(), wall.GetWidth()) / 2;
}
