#pragma once

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "request_handler.h"

#include <vector>

class JsonReader {
 public:
  /**
   * Парсит входящий поток и заполняет массивы base_requests_
   */
  void ParseStream(std::istream &ist);

  /**
   * Наполняет транспортный справочник данными, используя запросы из base_requests_
   */
  void FillCatalogue(tc::TransportCatalogue &catalogue) const;

  /**
   * Наполняет визуализатор карты данными, используя запросы из base_requests_
   */
  const renderer::Params &FillRenderSettings() const;

  /**
   * Выводит данные в поток.
   * В качестве аргументов требует RequestHandler, являющийся оболочкой между системами "Каталог" и "Рендер"
   */
  void ParseRequests(const RequestHandler &handler, std::ostream &out) const;

 private:
  void ParseBaseRequests(const json::Array &requests);
  void ParseStatRequests(const json::Array &requests);
  void ParseRenderSettings(const json::Dict &requests);

  json::Node node_ = nullptr;

  std::vector<BaseRequestDescription> base_requests_;
  std::vector<StatRequestDescription> stat_requests_;
  renderer::Params render_settings_;
};