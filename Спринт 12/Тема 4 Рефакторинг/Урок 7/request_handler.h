#pragma once

#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <optional>
#include <unordered_set>

using tc::TransportCatalogue;
using tc::RouteInfo;

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

enum class TypeRequest {
  qRoute,
  qStop,
  qMap,
  qPath
};

struct BaseRequestDescription {
  explicit operator bool() const {
    return !name.empty();
  }

  bool operator!() const {
    return !operator bool();
  }

  TypeRequest type;
  std::string name;
  std::vector<std::string_view> stops;
  bool is_roundtrip;
  geo::Coordinates coordinates;
  std::map<std::string_view, int> distances;
};

struct StatRequestDescription {
  explicit operator bool() const {
    return !name.empty();
  }

  bool operator!() const {
    return !operator bool();
  }

  int id;
  TypeRequest type;
  std::string name;
  std::string path_from;
  std::string path_to;
};

class RequestHandler {
 public:
  RequestHandler(const TransportCatalogue &db, const renderer::MapRenderer &renderer, const router::Router &router)
      : db_(db), renderer_(renderer), router_(router) {}

  RouteInfo GetRouteInfo(std::string_view name) const;

  const std::set<std::string_view> &GetRoutes(std::string_view stop_name) const;

  svg::Document RenderMap() const;

  router::RouteInfo FindRoute(std::string_view from, std::string_view to) const;

 private:
  const TransportCatalogue &db_;
  const renderer::MapRenderer &renderer_;
  const router::Router &router_;
};