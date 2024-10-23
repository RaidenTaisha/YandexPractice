#pragma once

#include "geo.h"

#include <deque>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace tc {

struct Stop {
  std::string name_;
  Coordinates coordinates_;
};

struct Route {
  std::string name_;
  std::vector<Stop *> stops_;
};

struct RouteInfo {
  size_t total_stops_{};
  size_t unique_stops_{};
  double length_{};
};

class TransportCatalogue {
 public:
  void AddStop(const std::string& name, const Coordinates& coordinates);
  void AddRoute(const std::string& name, const std::vector<std::string_view>& stops);

  Route *GetRoute(const std::string_view& name) const;
  Stop *GetStop(const std::string_view& name) const;
  std::unordered_set<std::string_view> GetRoutes(const std::string_view& stop_name) const;

  RouteInfo GetRouteInfo(const std::string_view& name) const;

 private:
  std::deque<Stop> stops_;
  std::deque<Route> routes_;
  std::unordered_map<std::string_view, Stop *> stopname_to_stop_;
  std::unordered_map<std::string_view, Route *> routename_to_route_;
  std::unordered_map<std::string_view, std::vector<Route *>> stopname_to_routes_;
};

}
