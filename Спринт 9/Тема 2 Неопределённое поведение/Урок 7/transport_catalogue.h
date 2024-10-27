#pragma once

#include "geo.h"

#include <deque>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <set>
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
  double direct_length_{};
  size_t real_length_{};
};

struct StopsPairHasher {
  size_t operator()(const std::pair<const Stop *, const Stop *> &stops) const {
    if (stops.first->coordinates_ != stops.second->coordinates_)
      return salt_ * hasher_(stops.first->name_) + hasher_(stops.second->name_);
    return hasher_(stops.first->name_) + hasher_(stops.second->name_);
  }

  // Расстояния между пунктами А и Б могут отличаться в зависимости от направления движения. Хэш пары остановок
  // вычисляется путём сложения хэшей двух остановок и является одинаковым для пар вида (А; Б) и (Б; А). Таким образом
  // необходимо умножать одно из слагаемых на произвольный коэффициент для предотвращения коллизий.
  static constexpr int salt_ = 10;
  std::hash<std::string> hasher_;
};

class TransportCatalogue {
 public:
  void AddStop(const std::string& name, const Coordinates& coordinates);
  void AddRoute(const std::string& name, const std::vector<std::string_view>& stops);
  void SetDistance(const std::pair<const Stop *, const Stop *> &stops, size_t distance);

  const Route *GetRoute(const std::string_view& name) const;
  const Stop *GetStop(const std::string_view& name) const;
  size_t GetDistance(const std::pair<const Stop *, const Stop *> &stops) const;
  const std::set<std::string_view>& GetRoutes(const std::string_view& stop_name) const;

  RouteInfo GetRouteInfo(const std::string_view& name) const;

 private:
  std::deque<Stop> stops_;
  std::deque<Route> routes_;
  std::unordered_map<std::string_view, Stop *> stopname_to_stop_;
  std::unordered_map<std::string_view, Route *> routename_to_route_;

  /* Я предпочел бы использовать в этом месте unordered_set, так как не было требований о выводе маршрутов в алфавитном
   * порядке. Но с unordered_set этот код не проходит тесты, потому что в тестах вывод отсортирован в алфавитном
   * порядке 🙂.*/
  std::unordered_map<std::string_view, std::set<std::string_view>> stopname_to_routenames_;
  std::unordered_map<std::pair<const Stop *, const Stop *>, size_t, StopsPairHasher> distances_;
};

}
