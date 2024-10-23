#include "transport_catalogue.h"

#include <stdexcept>

using namespace tc;

void TransportCatalogue::AddStop(const std::string& name, const Coordinates& coordinates) {
  stops_.push_back({name, coordinates});
  stopname_to_stop_.insert({stops_.back().name_, &stops_.back()});
}

void TransportCatalogue::AddRoute(const std::string& name, const std::vector<std::string_view>& stops) {
  Route route{name, {}};

  for (const auto& stop_name : stops) {
    auto *find_stop = stopname_to_stop_.at(stop_name);
    route.stops_.push_back(find_stop);
  }

  routes_.push_back(std::move(route));
  routename_to_route_.insert({routes_.back().name_, &routes_.back()});

  for (const auto& stop : routes_.back().stops_)
    stopname_to_routenames_[stop->name_].insert(routes_.back().name_);
}

const Route *TransportCatalogue::GetRoute(const std::string_view& name) const {
  const auto it = routename_to_route_.find(name);
  return it == routename_to_route_.end() ? nullptr : it->second;
}

const Stop *TransportCatalogue::GetStop(const std::string_view& name) const {
  const auto it = stopname_to_stop_.find(name);
  return it == stopname_to_stop_.end() ? nullptr : it->second;
}

const std::unordered_set<std::string_view>& TransportCatalogue::GetRoutes(const std::string_view& stop_name) const {
  return stopname_to_routenames_.at(stop_name);
}

size_t CountUniqueStops(const std::vector<Stop *>& stops) {
  std::unordered_set<std::string_view> names;
  for (const auto& stop : stops)
    names.insert(stop->name_);
  return names.size();
}

double ComputeDistanceRoute(const std::vector<Stop *>& stops) {
  double route_distance = 0;
  for (size_t begin = 0, end = stops.size(); begin < (end - 1); ++begin)
    route_distance += ComputeDistance(stops[begin]->coordinates_, stops[begin + 1]->coordinates_);
  return route_distance;
}

RouteInfo TransportCatalogue::GetRouteInfo(const std::string_view& name) const {
  const auto route = routename_to_route_.at(name);
  return RouteInfo{route->stops_.size(), CountUniqueStops(route->stops_), ComputeDistanceRoute(route->stops_)};
}
