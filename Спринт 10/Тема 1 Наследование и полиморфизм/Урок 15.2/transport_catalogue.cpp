#include "transport_catalogue.h"

#include <algorithm>

using namespace tc;

void TransportCatalogue::AddStop(const std::string &name, const geo::Coordinates &coordinates) {
  stops_.push_back({name, coordinates, {}});
  stopname_to_stop_.insert({stops_.back().name_, &stops_.back()});
}

void TransportCatalogue::AddRoute(const std::string &name,
                                  const std::vector<std::string_view> &stops,
                                  bool is_rounded) {
  Route route{name, {}, is_rounded};
  routes_.push_back(std::move(route));

  for (const auto &stop_name : stops) {
    auto *find_stop = stopname_to_stop_.at(stop_name);
    find_stop->routes_.push_back(&routes_.back());
    routes_.back().stops_.push_back(find_stop);
  }

  routename_to_route_.insert({routes_.back().name_, &routes_.back()});

  for (const auto &stop : routes_.back().stops_)
    stopname_to_routenames_[stop->name_].insert(routes_.back().name_);
}

const Route *TransportCatalogue::GetRoute(const std::string_view &name) const {
  const auto it = routename_to_route_.find(name);
  return it == routename_to_route_.end() ? nullptr : it->second;
}

const Stop *TransportCatalogue::GetStop(const std::string_view &name) const {
  const auto it = stopname_to_stop_.find(name);
  return it == stopname_to_stop_.end() ? nullptr : it->second;
}

const std::set<std::string_view> &TransportCatalogue::GetRoutes(const std::string_view &stop_name) const {
// TODO Обработка случая когда остановка не входит ни в один из маршрутов
//  static std::set<std::string_view> empty;
//  if (GetStop(stop_name) == nullptr) {
//    // Если такой остановки нет, метод at выкинет исключение за нас, которое будет обработано
//    return stopname_to_routenames_.at(stop_name);
//  } else if (stopname_to_routenames_.find(stop_name) == stopname_to_routenames_.end()) {
//    // Если остановка есть, но через неё не проходит ни один маршрут, возвращаем пустой список маршрутов
//    return empty;
//  } else
  return stopname_to_routenames_.at(stop_name);
}

size_t CountUniqueStops(const std::vector<const Stop *> &stops) {
  std::unordered_set<std::string_view> names;
  for (const auto &stop : stops)
    names.insert(stop->name_);
  return names.size();
}

double ComputeLineDistanceRoute(const std::vector<Stop *> &stops) {
  double route_distance = 0;
  for (size_t begin = 0, end = stops.size(); begin < (end - 1); ++begin)
    route_distance += ComputeDistance(stops.at(begin)->coordinates_, stops.at(begin + 1)->coordinates_);
  return route_distance;
}

double ComputeDirectDistanceRoute(const std::vector<const Stop *> &stops) {
  double route_distance = 0;
  for (size_t begin = 0, end = stops.size(); begin < (end - 1); ++begin)
    route_distance += ComputeDistance(stops.at(begin)->coordinates_, stops.at(begin + 1)->coordinates_);
  return route_distance;
}

RouteInfo TransportCatalogue::GetRouteInfo(const std::string_view &name) const {
  const auto route = routename_to_route_.at(name);
  size_t real_route_length = 0;
  for (size_t i = 0; i < route->stops_.size() - 1; ++i)
    real_route_length += GetDistance({route->stops_.at(i), route->stops_.at(i + 1)});

  return {route->stops_.size(), CountUniqueStops(route->stops_), ComputeDirectDistanceRoute(route->stops_),
          real_route_length};
}

void TransportCatalogue::SetDistance(const std::pair<const Stop *, const Stop *> &stops, size_t distance) {
  distances_.insert({stops, distance});
}

size_t TransportCatalogue::GetDistance(const std::pair<const Stop *, const Stop *> &stops) const {
  auto it = distances_.find(stops);

  if (it == distances_.end()) {
    it = distances_.find({stops.second, stops.first});
    if (it == distances_.end()) {
      return std::numeric_limits<size_t>::max();
    }
  }

  return it->second;
}

std::vector<const Route *> TransportCatalogue::GetSortedAllNonEmptyRoutes() const {
  std::vector<const Route *> ret;
  ret.reserve(routes_.size());
  for (const auto &route : routes_)
    if (!route.stops_.empty()) ret.push_back(&route);
  std::sort(ret.begin(), ret.end(), [](const auto lhs, const auto rhs) { return lhs->name_ < rhs->name_; });
  return ret;

}

std::vector<const Stop *> TransportCatalogue::GetSortedAllNonEmptyStops() const {
  std::vector<const Stop *> ret;
  ret.reserve(stops_.size());
  for (const auto &stop : stops_)
    if (!stop.routes_.empty()) ret.push_back(&stop);
  std::sort(ret.begin(), ret.end(), [](const auto lhs, const auto rhs) { return lhs->name_ < rhs->name_; });
  return ret;
}
