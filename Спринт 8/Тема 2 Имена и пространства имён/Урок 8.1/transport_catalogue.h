#pragma once

#include <list>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <utility>

#include "geo.h"

struct Stop {
  std::string name_;
  Coordinates coordinates_;
};

struct RouteInfo {
  size_t total_stops{};
  size_t unique_stops{};
  double route_length{};
};

struct Route {
  std::string name_;
  std::vector<Stop> stops_;
};

class TransportCatalogue {
 public:
  void AddStop(const Stop &stop);
  void AddRoute(const Route &route);

  const Stop &GetStop(const std::string_view &stop_name) const;
  const Route &GetRoute(const std::string_view &route_name) const;
  std::vector<Route> GetRoutes(const std::string_view &stop_name) const;
  RouteInfo GetRouteInfo(const std::string_view &route_name) const;

 private:
  std::vector<Stop> stops_;
  std::vector<Route> routes_;
  std::unordered_map<std::string, Stop> name_to_stop_;
  std::unordered_map<std::string, Route> name_to_route_;
};
