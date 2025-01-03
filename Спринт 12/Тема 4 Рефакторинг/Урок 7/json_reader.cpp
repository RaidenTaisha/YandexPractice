#include "json_reader.h"

#include <algorithm>
#include <sstream>

namespace {

void BuildRouteItem(json::Array &result, const router::RouteInfo::Moving &item) {
  result.emplace_back(json::Builder{}.
    StartDict().
      Key("type").Value("Bus").
      Key("bus").Value(std::string(item.routename)).
      Key("time").Value(item.time.count()).
      Key("span_count").Value(static_cast<int>(item.steps_count)).
    EndDict().
  Build());
}

void BuildRouteItem(json::Array &result, const router::RouteInfo::Waiting &item) {
  result.emplace_back(json::Builder{}.
    StartDict().
      Key("type").Value("Wait").
      Key("stop_name").Value(std::string(item.stopname)).
      Key("time").Value(item.time.count()).
    EndDict().
  Build());
}

}

void JsonReader::ParseBaseRequests(const json::Array &requests) {
  for (const auto &request : requests) {
    BaseRequestDescription request_description;
    const auto &request_map = request.AsDict();
    request_description.name = request_map.at("name").AsString();
    if (request_map.at("type").AsString() == "Bus") {
      request_description.type = TypeRequest::qRoute;
      for (const auto &stop : request_map.at("stops").AsArray()) {
        request_description.stops.push_back(stop.AsString());
      }
      request_description.is_roundtrip = request_map.at("is_roundtrip").AsBool();
      if (!request_description.is_roundtrip) {
        size_t i = request_description.stops.size() - 1;
        while (i > 0) {
          --i;
          request_description.stops.push_back(request_description.stops.at(i));
        }
      }
    } else {
      request_description.type = TypeRequest::qStop;
      request_description.coordinates.lat = request_map.at("latitude").AsDouble();
      request_description.coordinates.lng = request_map.at("longitude").AsDouble();
      for (const auto &stop : request_map.at("road_distances").AsDict()) {
        request_description.distances.insert({stop.first, stop.second.AsInt()});
      }
    }
    base_requests_.push_back(std::move(request_description));
  }
}

svg::Color SerializeColor(const json::Node &request) {
  if (request.IsString()) {
    return request.AsString();
  }

  if (request.IsArray()) {
    const auto &color_array = request.AsArray();
    if (color_array.size() == 3) {
      return svg::Rgb{static_cast<unsigned char>(color_array.at(0).AsInt()),
                      static_cast<unsigned char>(color_array.at(1).AsInt()),
                      static_cast<unsigned char>(color_array.at(2).AsInt())};
    }
    if (color_array.size() == 4) {
      return svg::Rgba{static_cast<unsigned char>(color_array.at(0).AsInt()),
                       static_cast<unsigned char>(color_array.at(1).AsInt()),
                       static_cast<unsigned char>(color_array.at(2).AsInt()),
                       color_array.at(3).AsDouble()};
    }
  }

  return svg::NoneColor;
}

void JsonReader::ParseRenderSettings(const json::Dict &request) {
  render_settings_.width_ = request.at("width").AsDouble();
  render_settings_.height_ = request.at("height").AsDouble();

  render_settings_.padding_ = request.at("padding").AsDouble();

  render_settings_.line_width_ = request.at("line_width").AsDouble();
  render_settings_.stop_radius_ = request.at("stop_radius").AsDouble();

  render_settings_.route_label_font_size_ = static_cast<unsigned>(request.at("bus_label_font_size").AsInt());
  render_settings_.route_label_offset_ = {
      request.at("bus_label_offset").AsArray().at(0).AsDouble(),
      request.at("bus_label_offset").AsArray().at(1).AsDouble()
  };

  render_settings_.stop_label_font_size_ = static_cast<unsigned>(request.at("stop_label_font_size").AsInt());
  render_settings_.stop_label_offset_ = {
      request.at("stop_label_offset").AsArray().at(0).AsDouble(),
      request.at("stop_label_offset").AsArray().at(1).AsDouble()
  };

  render_settings_.underlayer_color_ = SerializeColor(request.at("underlayer_color"));
  render_settings_.underlayer_width_ = request.at("underlayer_width").AsDouble();

  for (const auto &color : request.at("color_palette").AsArray()) {
    render_settings_.color_palette_.push_back(SerializeColor(color));
  }
}

void JsonReader::ParseStatRequests(const json::Array &requests) {
  for (const auto &request : requests) {
    StatRequestDescription new_request;
    const auto &request_map = request.AsDict();
    new_request.id = request_map.at("id").AsInt();

    const auto &type_request = request_map.at("type").AsString();
    if (type_request == "Bus") {
      new_request.type = TypeRequest::qRoute;
      new_request.name = request_map.at("name").AsString();
    } else if (type_request == "Stop") {
      new_request.type = TypeRequest::qStop;
      new_request.name = request_map.at("name").AsString();
    } else if (type_request == "Map") {
      new_request.type = TypeRequest::qMap;
    } else if (type_request == "Route") {
      new_request.type = TypeRequest::qPath;
      new_request.path_from = request_map.at("from").AsString();
      new_request.path_to = request_map.at("to").AsString();
    }
    stat_requests_.push_back(std::move(new_request));
  }
}

void JsonReader::ParseRouterSettings(const json::Dict &requests) {
  router_settings_.bus_wait_time = std::chrono::minutes(requests.at("bus_wait_time").AsInt());
  router_settings_.bus_velocity = requests.at("bus_velocity").AsDouble();
}

void JsonReader::ParseStream(std::istream &ist) {
  node_ = json::Load(ist).GetRoot();
  for (const auto &[key, value] : node_.AsDict()) {
    if (key == "base_requests") {
      ParseBaseRequests(value.AsArray());
    } else if (key == "stat_requests") {
      ParseStatRequests(value.AsArray());
    } else if (key == "render_settings") {
      ParseRenderSettings(value.AsDict());
    } else if (key == "routing_settings") {
      ParseRouterSettings(value.AsDict());
    }
  }
}

void JsonReader::FillCatalogue(tc::TransportCatalogue &catalogue) const {
  std::vector sorted_commands(base_requests_.begin(), base_requests_.end());
  std::sort(sorted_commands.begin(),
            sorted_commands.end(),
            [](const BaseRequestDescription &lhs, const BaseRequestDescription &rhs) {
              return lhs.type > rhs.type;
            });

  for (const auto &command : sorted_commands) {
    if (command.type == TypeRequest::qStop) {
      catalogue.AddStop(command.name, command.coordinates);
    } else if (command.type == TypeRequest::qRoute) {
      catalogue.AddRoute(command.name, command.stops, command.is_roundtrip);
    }
  }
  for (const auto &command : sorted_commands) {
    if (command.type == TypeRequest::qStop) {
      for (const auto distance : command.distances) {
        catalogue.SetDistance({catalogue.GetStop(command.name), catalogue.GetStop(distance.first)},
                              static_cast<size_t>(distance.second));
      }
    }
  }
}

const renderer::Params &JsonReader::FillRenderSettings() const {
  return render_settings_;
}

const router::Params &JsonReader::FillRouterSettings() const {
  return router_settings_;
}

void JsonReader::ParseRequests(const RequestHandler &handler, std::ostream &out) const {
  std::stringstream ss;
  json::Array result;
  for (const auto &[id, type, name, from, to] : stat_requests_) {
    switch (type) {
      case TypeRequest::qRoute:
        try {
          const auto route_info = handler.GetRouteInfo(name);
          result.emplace_back(json::Builder{}.
            StartDict().
              Key("request_id").Value(id).
              Key("curvature").Value(static_cast<double>(route_info.real_length_) / route_info.direct_length_).
              Key("route_length").Value(static_cast<int>(route_info.real_length_)).
              Key("stop_count").Value(static_cast<int>(route_info.total_stops_)).
              Key("unique_stop_count").Value(static_cast<int>(route_info.unique_stops_)).
            EndDict().
          Build());
        } catch (const std::out_of_range &) {
          result.emplace_back(json::Builder{}.
            StartDict().
              Key("request_id").Value(id).
              Key("error_message").Value("not found").
            EndDict().
          Build());
        }
        break;
      case TypeRequest::qStop:
        try {
          json::Array routes;
          for (const auto &route : handler.GetRoutes(name)) {
            routes.emplace_back(std::string(route));
          }
          result.emplace_back(json::Builder{}.
            StartDict().
              Key("request_id").Value(id).
              Key("buses").Value(routes).
            EndDict().
          Build());
        } catch (const std::out_of_range &) {
          result.emplace_back(json::Builder{}.
            StartDict().
              Key("request_id").Value(id).
              Key("error_message").Value("not found").
            EndDict().
          Build());
        }
        break;
      case TypeRequest::qMap:
        handler.RenderMap().Render(ss);
        result.emplace_back(json::Builder{}.
          StartDict().
            Key("request_id").Value(id).
            Key("map").Value(ss.str()).
          EndDict().
        Build());
        break;
      case TypeRequest::qPath:
        try {
          const auto &routing = handler.FindRoute(from, to);
          json::Array items;
          for (const auto &item : routing.items) {
            std::visit([&items](const auto &item) { BuildRouteItem(items, item); }, item);
          }
          result.emplace_back(json::Builder{}.
            StartDict().
              Key("request_id").Value(id).
              Key("total_time").Value(routing.total_time.count()).
              Key("items").Value(items).
            EndDict().
          Build());
        } catch (const std::exception &) {
          result.emplace_back(json::Builder{}.
              StartDict().
              Key("request_id").Value(id).
              Key("error_message").Value("not found").
              EndDict().
              Build());
        }
        break;
      default:
        // Недостижимая ветка
        __builtin_unreachable();
    }
  }
  json::Print(json::Document{result}, out);
}
