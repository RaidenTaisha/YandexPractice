#pragma once

#include "map_renderer.h"
#include "transport_catalogue.h"

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
  qMap
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
};

class RequestHandler {
 public:
  RequestHandler(const TransportCatalogue &db, const renderer::MapRenderer &renderer)
      : db_(db), renderer_(renderer) {}

  RouteInfo GetRouteInfo(std::string_view name) const;

  const std::set<std::string_view> &GetRoutes(std::string_view stop_name) const;

  svg::Document RenderMap() const;

 private:
  const TransportCatalogue &db_;
  const renderer::MapRenderer &renderer_;
};