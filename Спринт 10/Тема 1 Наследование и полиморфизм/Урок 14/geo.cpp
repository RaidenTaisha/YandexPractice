#include "geo.h"

#include <cmath>

namespace geo {

bool Coordinates::operator==(const Coordinates& other) const {
  return lat == other.lat && lng == other.lng;
}
bool Coordinates::operator!=(const Coordinates& other) const {
  return !(*this == other);
}

double ComputeDistance(Coordinates from, Coordinates to) {
  using namespace std;
  if (from == to) {
    return 0;
  }
  static const double rad = M_PI / 180.;
  static const double earth_radius = 6371000;
  return acos(sin(from.lat * rad) * sin(to.lat * rad)
                  + cos(from.lat * rad) * cos(to.lat * rad)
                      * cos(abs(from.lng - to.lng) * rad)) * earth_radius;
}

}  // namespace geo
