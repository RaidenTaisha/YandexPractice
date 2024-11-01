#include "domain.h"

using namespace tc;

size_t StopsPairHasher::operator()(const std::pair<const Stop *, const Stop *>& stops) const {
  if (stops.first->coordinates_ != stops.second->coordinates_)
    return salt_ * hasher_(stops.first->name_) + hasher_(stops.second->name_);
  return hasher_(stops.first->name_) + hasher_(stops.second->name_);
}
