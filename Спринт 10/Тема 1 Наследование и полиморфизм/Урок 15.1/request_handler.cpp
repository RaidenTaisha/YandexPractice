#include "request_handler.h"
#include "svg.h"

#include <sstream>

namespace {
svg::Color SerializeColor(const json::Node& node) {
  if (node.IsArray()) {
    const auto& arr = node.AsArray();

    if (arr.size() == 3) {
      return svg::Rgb{static_cast<unsigned char>(arr[0].AsInt()),
          static_cast<unsigned char>(arr[1].AsInt()),
          static_cast<unsigned char>(arr[2].AsInt())};
    } else if (arr.size() == 4) {
      return svg::Rgba{static_cast<unsigned char>(arr[0].AsInt()),
          static_cast<unsigned char>(arr[1].AsInt()),
          static_cast<unsigned char>(arr[2].AsInt()),
          arr[3].AsDouble(),};
    }
  } else if (node.IsString()) {
    return node.AsString();
  }

  return svg::NoneColor;
}

svg::Point SerializePoint(const json::Array& arr) {
  return {arr[0].AsDouble(), arr[1].AsDouble()};
}

std::vector<svg::Color> SerializeColors(const json::Array& arr) {
  std::vector<svg::Color> colors;
  colors.reserve(arr.size());

  for (const auto& color : arr)
    colors.emplace_back(SerializeColor(color));

  return colors;
}

} // namespace

RequestHandler::RequestHandler(const tc::TransportCatalogue& db) : db_(db) {}

tc::RouteInfo RequestHandler::GetRouteInfo(const std::string_view& name) const {
  return db_.GetRouteInfo(name);
}

const std::set<std::string_view>& RequestHandler::GetRoutes(const std::string_view& stop_name) const {
  return db_.GetRoutes(stop_name);
}

json::Document RequestHandler::ProcessingRequests(const json::Document& doc) const {
  using namespace std::string_literals;

  json::Array output_json;
  json::Dict response;

  json::Array stat_requests = doc.GetRoot().AsMap().at("stat_requests").AsArray();
  for (const auto& req : stat_requests) {
    auto map_req = req.AsMap();
    response.emplace("request_id", map_req.at("id").AsInt());

    if (map_req.at("type") == "Stop"s)
      response.merge(GetStopStat(map_req.at("name").AsString()));
    else if (map_req.at("type") == "Bus"s)
      response.merge(GetRouteStat(map_req.at("name").AsString()));
  }
  response.merge(GetMap(doc.GetRoot().AsMap().at("render_settings").AsMap()));

  output_json.emplace_back(response);
  return json::Document{output_json};
}

json::Dict RequestHandler::GetRouteStat(const std::string_view id) const {
  using namespace std::string_literals;
  json::Dict ret;
  const auto bus = db_.GetRoute(id);

  if (nullptr == bus) {
    ret.emplace("error_message"s, "not found");
  } else {
    /* Я считаю что тут не нужен блок try {} catch () {}, потому что непонятно какой результат возвращать пользователю
       в случе если такой остановки нет. Я трактую такой вариант как возникновение исключетельной ситуации, и считаю
       справедливым выход программы из строя. Ибо нефиг кормить программу абсурдными данными */
//      try {
    const auto info = db_.GetRouteInfo(id);
    ret.emplace("curvature", static_cast<double>(info.real_length_) / info.direct_length_);
    ret.emplace("route_length", static_cast<double>(info.real_length_));
    ret.emplace("stop_count", static_cast<int>(info.total_stops_));
    ret.emplace("unique_stop_count", static_cast<int>(info.unique_stops_));
//      } catch (const std::out_of_range &e) {}
  }

  return ret;
}

json::Dict RequestHandler::GetStopStat(const std::string_view id) const {
  using namespace std::string_literals;
  json::Dict ret;

  const auto *stop = db_.GetStop(id);
  if (nullptr == stop) {
    ret.emplace("error_message"s, "not found");
  } else {
    try {
      json::Array buses;
      for (const auto& bus : db_.GetRoutes(id))
        buses.emplace_back(std::string(bus));
      ret.emplace("buses", buses);
    } catch (const std::out_of_range&) {
      // Если через остановку не проходит ни один маршрут, то возвращаем пустой массив
      ret.emplace("buses", json::Array{});
    }
  }
  return ret;
}
json::Dict RequestHandler::GetMap(const json::Dict& params) const {
  using namespace std::string_view_literals;

  json::Dict ret;
  svg::Document doc;
  std::stringstream render;

  tc::renderer::Map m({params.at("height").AsDouble(),
                          params.at("line_width").AsDouble(),
                          params.at("padding").AsDouble(),
                          params.at("stop_radius").AsDouble(),
                          params.at("underlayer_width").AsDouble(),
                          params.at("width").AsDouble(),
                          static_cast<unsigned int>(params.at("bus_label_font_size").AsInt()),
                          static_cast<unsigned int>(params.at("stop_label_font_size").AsInt()),
                          SerializeColors(params.at("color_palette").AsArray()),
                          SerializePoint(params.at("bus_label_offset").AsArray()),
                          SerializePoint(params.at("stop_label_offset").AsArray()),
                          SerializeColor(params.at("underlayer_color"))}, db_.GetAllRoutes());
  m.Draw(doc);
  doc.Render(render);
  ret.emplace("map", render.str());

  return ret;
}
