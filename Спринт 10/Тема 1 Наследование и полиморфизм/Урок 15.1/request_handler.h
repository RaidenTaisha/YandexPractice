#pragma once

#include "domain.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

class RequestHandler {
 public:
  // MapRenderer понадобится в следующей части итогового проекта
  RequestHandler(const tc::TransportCatalogue& db);

  // Возвращает информацию о маршруте (запрос Bus)
  tc::RouteInfo GetRouteInfo(const std::string_view& name) const;

  // Возвращает маршруты, проходящие через остановку (запрос Stop)
  const std::set<std::string_view>& GetRoutes(const std::string_view& stop_name) const;

  json::Document ProcessingRequests(const json::Document& doc) const;

  json::Dict GetStopStat(std::string_view id) const;

  json::Dict GetRouteStat(std::string_view id) const;

  json::Dict GetMap(const json::Dict& settings) const;

  svg::Document RenderMap() const;

 private:
  // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
  const tc::TransportCatalogue& db_;
};
