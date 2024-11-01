#include "json_reader.h"
#include "json.h"

#include <string>
#include <string_view>
#include <unordered_map>

namespace {

std::unordered_map<std::string, json::Dict> SerializeStops(tc::TransportCatalogue& catalogue, json::Array& data) {
  using namespace std::string_literals;

  std::unordered_map<std::string, json::Dict> ret;
  for (const auto& req : data) {
    auto map_req = req.AsMap();
    auto type_stop = map_req.at("type"s);

    if (type_stop == "Stop"s) {
      auto name = map_req.at("name"s).AsString();
      auto lat = map_req.at("latitude"s).AsDouble();
      auto lng = map_req.at("longitude"s).AsDouble();
      ret.emplace(name, map_req.at("road_distances"s).AsMap());

      catalogue.AddStop(name, {lat, lng});
    }
  }
  return ret;
}

void SerializeDistances(tc::TransportCatalogue& catalogue, std::unordered_map<std::string, json::Dict>& distances) {
  for (const auto& [stop_a_name, map_distances] : distances) {
    for (const auto& [stop_b_name, dist] : map_distances) {
      auto a = catalogue.GetStop(stop_a_name);
      auto b = catalogue.GetStop(stop_b_name);
      catalogue.SetDistance({a, b}, static_cast<size_t>(dist.AsInt()));
    }
  }
}

void SerializeRoutes(tc::TransportCatalogue& catalogue, json::Array& data) {
  using namespace std::string_literals;

  for (const auto& req : data) {
    auto map_req = req.AsMap();

    if (map_req.at("type"s) == "Bus"s) {
      std::vector<std::string_view> stops;

      for (const auto& stop : map_req.at("stops"s).AsArray()) {
        stops.push_back(stop.AsString());
      }

      if (!map_req.at("is_roundtrip"s).AsBool()) {
        auto copy = stops;
        stops.insert(stops.end(), ++copy.rbegin(), copy.rend());
      }

      catalogue.AddRoute(map_req.at("name"s).AsString(), stops);
    }
  }
}

} // namesapce

void tc::SerializeToCatalogue(TransportCatalogue& catalogue, const json::Document& doc) {
  json::Array base_requests = doc.GetRoot().AsMap().at("base_requests").AsArray();

  // Добавление остановок в базу данных
  auto distances = SerializeStops(catalogue, base_requests);

  // Добавление расстояний между остановками в базу данных
  SerializeDistances(catalogue, distances);

  // Добавление маршрутов в базу данных
  SerializeRoutes(catalogue, base_requests);
}
