#pragma once

#include "domain.h"
#include "geo.h"

#include <deque>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>

namespace tc {

class TransportCatalogue {
 public:
  void AddStop(const std::string& name, const geo::Coordinates& coordinates);
  void AddRoute(const std::string& name, const std::vector<std::string_view>& stops, bool is_rounded);
  void SetDistance(const std::pair<const Stop *, const Stop *>& stops, size_t distance);

  const Route *GetRoute(const std::string_view& name) const;
  const Stop *GetStop(const std::string_view& name) const;
  size_t GetDistance(const std::pair<const Stop *, const Stop *>& stops) const;
  const std::set<std::string_view>& GetRoutes(const std::string_view& stop_name) const;
  std::vector<const Route*> GetSortedAllNonEmptyRoutes() const;
  std::vector<const Stop*> GetSortedAllNonEmptyStops() const;

  RouteInfo GetRouteInfo(const std::string_view& name) const;

 private:
  std::deque<Stop> stops_;
  std::deque<Route> routes_;
  std::unordered_map<std::string_view, Stop *> stopname_to_stop_;
  std::unordered_map<std::string_view, Route *> routename_to_route_;

  std::unordered_map<std::string_view, std::set<std::string_view>> stopname_to_routenames_;
  std::unordered_map<std::pair<const Stop *, const Stop *>, size_t, StopsPairHasher> distances_;
};

}
