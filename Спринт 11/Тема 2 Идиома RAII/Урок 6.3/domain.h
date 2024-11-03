#pragma once

#include "geo.h"

#include <cstddef>
#include <utility>
#include <vector>
#include <string>

namespace tc {

struct Route;
struct Stop {
  std::string name_;
  geo::Coordinates coordinates_;
  std::vector<const Route *> routes_;
};

struct Route {
  std::string name_;
  std::vector<const Stop *> stops_;
  bool is_rounded;
};

struct RouteInfo {
  size_t total_stops_{};
  size_t unique_stops_{};
  double direct_length_{};
  size_t real_length_{};
};

struct StopsPairHasher {
  size_t operator()(const std::pair<const Stop *, const Stop *>& stops) const;

  // Расстояния между пунктами А и Б могут отличаться в зависимости от направления движения. Хэш пары остановок
  // вычисляется путём сложения хэшей двух остановок и является одинаковым для пар вида (А; Б) и (Б; А). Таким образом
  // необходимо умножать одно из слагаемых на произвольный коэффициент для предотвращения коллизий.
  static constexpr int salt_ = 10;
  std::hash<std::string> hasher_;
};

}  // namespace tc
