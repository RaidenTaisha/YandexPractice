#include "geo.h"
#include "map_renderer.h"

#include <utility>
#include <unordered_set>

namespace tc::renderer {

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
  return {(coords.lng - min_lon_) * zoom_coeff_ + padding_, (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
}

Map::Map(tc::renderer::Params params, const std::deque<tc::Route>& routes)
    : params_(std::move(params)), routes_(routes) {
  std::unordered_set<const Stop *> stops;
  for (const auto& route : routes_) {
    stops.insert(route.stops_.begin(), route.stops_.end());
  }

  std::vector<geo::Coordinates> positions;
  positions.reserve(stops.size());

  for (const auto stop : stops) {
    positions.push_back(stop->coordinates_);
  }

  SphereProjector projector{positions.begin(), positions.end(), params_.width_, params_.height_, params_.padding_};
  for (const auto stop : stops) {
    stop_to_coordinates_.emplace(stop->name_, projector(stop->coordinates_));
  }
}

void Map::Draw(svg::ObjectContainer& container) const {
  RenderBusLines(container);
//  RenderBusLabels(container);
//  RenderStops(container);
//  RenderStopLabels(container);
}

void Map::RenderBusLines(svg::ObjectContainer& container) const {
  size_t bus_index = 0;

  std::vector<Route> routes(routes_.begin(), routes_.end());
  std::sort(routes.begin(), routes.end(), [](const auto lhs, const auto rhs){ return lhs.name_ < rhs.name_; });
  for (const auto& route : routes) {
    const auto& stops = route.stops_;

    if (!stops.empty()) {
      auto line = svg::Polyline()
          .SetStrokeColor(GetBusLineColor(bus_index++))
          .SetStrokeWidth(params_.line_width_)
          .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
          .SetFillColor(svg::NoneColor)
          .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

      for (const auto *stop : stops) {
        line.AddPoint(stop_to_coordinates_.at(stop->name_));
      }
      container.Add(std::move(line));
    }
  }
}

void Map::RenderBusLabels(svg::ObjectContainer& container) const {
  size_t bus_index = 0;

  for (const auto& route : routes_) {
    if (route.stops_.empty()) {
      continue;
    }

    auto& bus_color = GetBusLineColor(bus_index++);

    for (const auto *stop : route.stops_) {
      auto& stop_pos = stop_to_coordinates_.at(stop->name_);
      auto base = svg::Text()
          .SetPosition(stop_pos)
          .SetOffset(params_.bus_label_offset_)
          .SetFontSize(params_.bus_label_font_size_)
          .SetFontFamily(params_.bus_label_font_family_)
          .SetFontWeight("bold")
          .SetData(route.name_);

      container.Add(svg::Text{base}
                        .SetFillColor(params_.underlayer_color_)
                        .SetStrokeColor(params_.underlayer_color_)
                        .SetStrokeWidth(params_.underlayer_width_)
                        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));

      container.Add(svg::Text{base}.SetFillColor(bus_color));
    }
  }
}

void Map::RenderStops(svg::ObjectContainer& container) const {
  for (auto it = stop_to_coordinates_.rbegin(); it != stop_to_coordinates_.rend(); it++) {
    const auto &stop_coord = it->second;
    container.Add(svg::Circle()
                      .SetRadius(params_.stop_radius_)
                      .SetCenter(stop_coord)
                      .SetFillColor("white"));
  }
}

void Map::RenderStopLabels(svg::ObjectContainer& container) const {
  for (auto it = stop_to_coordinates_.rbegin(); it != stop_to_coordinates_.rend(); it++) {
    const auto &stop = it->first;
    const auto &stop_coord = it->second;
    auto base = svg::Text()
        .SetPosition(stop_coord)
        .SetOffset(params_.stop_label_offset_)
        .SetFontSize(params_.stop_label_font_size_)
        .SetFontFamily(params_.stop_label_font_family_)
        .SetData(std::string(stop));
    container.Add(svg::Text{base}
                      .SetFillColor(params_.underlayer_color_)
                      .SetStrokeColor(params_.underlayer_color_)
                      .SetStrokeWidth(params_.underlayer_width_)
                      .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                      .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
    container.Add(svg::Text{base}.SetFillColor(params_.stop_label_color_));
  }
}

const svg::Color& Map::GetBusLineColor(size_t index) const {
  static const svg::Color default_color = svg::Color{"black"};
  const auto& palette = params_.color_palette_;

  return !palette.empty() ? palette.at(index % palette.size()) : default_color;
}

}