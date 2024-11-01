#include <algorithm>
#include <iomanip>
#include "json_reader.h"

void JsonReader::ParseBaseRequests(const json::Array &requests) {
  for (const auto &request : requests) {
    BaseRequestDescription request_description;
    const auto &request_map = request.AsMap();
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
      for (const auto &stop : request_map.at("road_distances").AsMap()) {
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
    const auto &request_map = request.AsMap();
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
    }
    stat_requests_.push_back(std::move(new_request));
  }
}

void JsonReader::ParseStream(std::istream &ist) {
  node_ = json::Load(ist).GetRoot();
  for (const auto &[key, value] : node_.AsMap()) {
    if (key == "base_requests") {
      ParseBaseRequests(value.AsArray());
    } else if (key == "stat_requests") {
      ParseStatRequests(value.AsArray());
    } else if (key == "render_settings") {
      ParseRenderSettings(value.AsMap());
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

void JsonReader::ParseRequests(const RequestHandler &handler, std::ostream &out) const {
  std::stringstream ss;
  json::Array result;
  for (const auto &[id, type, name] : stat_requests_) {
    switch (type) {
      case TypeRequest::qRoute:
        try {
          const auto route_info = handler.GetRouteInfo(name);
          result.emplace_back(json::Dict{
              {"request_id", json::Node(id)},
              {"curvature", json::Node(static_cast<double>(route_info.real_length_) / route_info.direct_length_)},
              {"route_length", json::Node(static_cast<int>(route_info.real_length_))},
              {"stop_count", json::Node(static_cast<int>(route_info.total_stops_))},
              {"unique_stop_count", json::Node(static_cast<int>(route_info.unique_stops_))},
          });
        } catch (const std::out_of_range &) {
          result.emplace_back(json::Dict{
              {"request_id", json::Node(id)},
              {"error_message", json::Node("not found")},
          });
        }
        break;
      case TypeRequest::qStop:
        try {
          json::Array routes;
          for (const auto &route : handler.GetRoutes(name)) {
            routes.emplace_back(std::string(route));
          }
          result.emplace_back(json::Dict{
              {"request_id", json::Node(id)},
              {"routes", routes},
          });
        } catch (const std::out_of_range &) {
          result.emplace_back(json::Dict{
              {"request_id", json::Node(id)},
              {"error_message", json::Node("not found")},
          });
        }
        break;
      case TypeRequest::qMap:
//      handler.RenderMap().Render(out);
//      handler.RenderMap().Render(ss);
//      result.emplace_back(json::Dict{
//          {"request_id", json::Node(id)},
//          {"map", json::Node(ss.str())},
//      });
        break;
      default:
        // Недостижимая ветка
//        __builtin_unreachable();
        break;
    }
  }
  handler.RenderMap().Render(out);
//  json::Print(json::Document{result}, out);
}
