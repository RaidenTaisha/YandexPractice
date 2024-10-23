#include "transport_catalogue.h"

#include <unordered_set>
#include <algorithm>

struct StopHasher {
  size_t operator()(const Stop &stop) const {
    return hasher(stop.name_);
  }

  std::hash<std::string> hasher;
};

bool operator==(const Stop &lhs, const Stop &rhs) {
  return lhs.name_ == rhs.name_;
}

void TransportCatalogue::AddStop(const Stop &stop) {
  stops_.push_back(stop);
  name_to_stop_.insert({stop.name_, stop});
}

void TransportCatalogue::AddRoute(const Route &route) {
  routes_.push_back(route);
  name_to_route_.insert({route.name_, route});
}

const Stop &TransportCatalogue::GetStop(const std::string_view &stop_name) const {
  return name_to_stop_.at(std::string(stop_name));
}

const Route &TransportCatalogue::GetRoute(const std::string_view &route_name) const {
  return name_to_route_.at(std::string(route_name));
}

RouteInfo TransportCatalogue::GetRouteInfo(const std::string_view &route_name) const {
  double route_distance = 0;
  const Route &route = GetRoute(std::string(route_name));

  // Уникальные остановки
  std::unordered_set<Stop, StopHasher> unique_stops(route.stops_.begin(), route.stops_.end());

  // Расчёт длины маршрута
  for (auto begin = route.stops_.begin(), end = --route.stops_.end(); begin != end;) {
    Coordinates first = begin->coordinates_;
    ++begin;
    Coordinates second = begin->coordinates_;
    route_distance += ComputeDistance(first, second);
  }

  return {route.stops_.size(), unique_stops.size(), route_distance};
}

std::vector<Route> TransportCatalogue::GetRoutes(const std::string_view &stop_name) const {
  std::vector<Route> routes;
  const auto &stop = name_to_stop_.at(std::string(stop_name));
  auto route = std::find_if(routes_.begin(),
                            routes_.end(),
                            [stop](auto it) {
                              return std::count(it.stops_.begin(), it.stops_.end(), stop) > 0;
                            });
  while (route != routes_.end()) {
    routes.push_back(*route);
    route = std::find_if(++route,
                         routes_.end(),
                         [stop](auto it) {
                           return std::count(it.stops_.begin(), it.stops_.end(), stop) > 0;
                         });
  }
  std::sort(routes.begin(), routes.end(), [](auto lhs, auto rhs) {
    return lhs.name_ < rhs.name_;
  });
  return routes;
}
