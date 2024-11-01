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
        stops.insert(stops.end(), copy.rbegin(), copy.rend());
      } else {
        stops.push_back(stops.front());
      }

      catalogue.AddRoute(map_req.at("name"s).AsString(), stops);
    }
  }
}

json::Dict PrintBusStat(const tc::TransportCatalogue& catalogue, const std::string_view id) {
  using namespace std::string_literals;
  json::Dict ret;
  const auto bus = catalogue.GetRoute(id);

  if (nullptr == bus) {
    ret.emplace("error_message", "not found");
  } else {
    /* Я считаю что тут не нужен блок try {} catch () {}, потому что непонятно какой результат возвращать пользователю
       в случе если такой остановки нет. Я трактую такой вариант как возникновение исключетельной ситуации, и считаю
       справедливым выход программы из строя. Ибо нефиг кормить программу абсурдными данными */
//      try {
    const auto info = catalogue.GetRouteInfo(id);
    ret.emplace("curvature", static_cast<double>(info.real_length_) / info.direct_length_);
    ret.emplace("route_length", static_cast<double>(info.real_length_));
    ret.emplace("stop_count", static_cast<int>(info.total_stops_));
    ret.emplace("unique_stop_count", static_cast<int>(info.unique_stops_));
//      } catch (const std::out_of_range &e) {}
  }

  return ret;
}

json::Dict PrintStopStat(const tc::TransportCatalogue& catalogue, const std::string_view id) {
  using namespace std::string_literals;
  json::Dict ret;

  const auto *stop = catalogue.GetStop(id);
  if (nullptr == stop) {
    ret.emplace("error_message", "not found");
  } else {
    try {
      json::Array buses;
      for (const auto& bus : catalogue.GetRoutes(id))
        buses.emplace_back(std::string(bus));
      ret.emplace("buses", buses);
    } catch (const std::out_of_range&) {
      ret.emplace("error_message", "no buses");
    }
  }
  return ret;
}

} // namesapce

void tc::SerializeToCatalogue(TransportCatalogue& catalogue, json::Document& doc) {
  json::Array base_requests = doc.GetRoot().AsMap().at("base_requests").AsArray();

  // Добавление остановок в базу данных
  auto distances = SerializeStops(catalogue, base_requests);

  // Добавление расстояний между остановками в базу данных
  SerializeDistances(catalogue, distances);

  // Добавление маршрутов в базу данных
  SerializeRoutes(catalogue, base_requests);
}

json::Document tc::ProcessingRequests(TransportCatalogue& catalogue, json::Document& doc) {
  using namespace std::string_literals;

  json::Array stat_requests = doc.GetRoot().AsMap().at("stat_requests").AsArray();
  json::Array output_json;

  for (const auto& req : stat_requests) {
    json::Dict response;
    auto map_req = req.AsMap();
    response.emplace("request_id", map_req.at("id"s).AsInt());

    if (map_req.at("type"s) == "Stop"s) {
      response.merge(PrintStopStat(catalogue, map_req.at("name"s).AsString()));
    } else if (map_req.at("type"s) == "Bus"s) {
      response.merge(PrintBusStat(catalogue, map_req.at("name"s).AsString()));
    }

    output_json.emplace_back(response);
  }

  return json::Document{output_json};
}
