#pragma once

#include "domain.h"
#include "geo.h"
#include "json.h"
#include "svg.h"

#include <deque>
#include <algorithm>

inline bool IsZero(double value) {
  static constexpr double epsilon = 1e-6;
  return std::abs(value) < epsilon;
}

namespace tc::renderer {

class SphereProjector {
 public:
  template <typename PointInputIt>
  SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                  double max_width, double max_height, double padding) : padding_(padding) {
    if (points_begin == points_end) {
      return;
    }

    const auto [left_it, right_it]
        = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
          return lhs.lng < rhs.lng;
        });
    min_lon_ = left_it->lng;
    max_lon_ = right_it->lng;

    const auto [bottom_it, top_it]
        = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
          return lhs.lat < rhs.lat;
        });
    min_lat_ = bottom_it->lat;
    max_lat_ = top_it->lat;

    std::optional<double> width_zoom;
    if (!IsZero(max_lon_ - min_lon_)) {
      width_zoom = (max_width - 2 * padding) / (max_lon_ - min_lon_);
    }

    std::optional<double> height_zoom;
    if (!IsZero(max_lat_ - min_lat_)) {
      height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat_);
    }

    if (width_zoom && height_zoom) {
      zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    } else if (width_zoom) {
      zoom_coeff_ = *width_zoom;
    } else if (height_zoom) {
      zoom_coeff_ = *height_zoom;
    }
  }

  svg::Point operator()(geo::Coordinates coords) const;

 private:
  double padding_{};
  double min_lon_{};
  double max_lon_{};
  double min_lat_{};
  double max_lat_{};
  double zoom_coeff_{};
};

struct Params {
  double height_;
  double line_width_;
  double padding_;
  double stop_radius_;
  double underlayer_width_;
  double width_;
  unsigned bus_label_font_size_;
  unsigned stop_label_font_size_;
  std::vector<svg::Color> color_palette_;
  svg::Point bus_label_offset_;
  svg::Point stop_label_offset_;
  svg::Color underlayer_color_{std::string("white")};
  std::string bus_label_font_family_{std::string("Verdana")};
  std::string stop_label_font_family_{std::string("Verdana")};
  svg::Color stop_label_color_{std::string("black")};
};

class Map : public svg::Drawable {
 public:
  Map(Params params, const std::deque<tc::Route>& routes);

  void Draw(svg::ObjectContainer& container) const override;

 private:
  void RenderBusLines(svg::ObjectContainer&) const;
  void RenderBusLabels(svg::ObjectContainer&) const;
  void RenderStops(svg::ObjectContainer&) const;
  void RenderStopLabels(svg::ObjectContainer&) const;

  const svg::Color& GetBusLineColor(size_t index) const;

  Params params_;
  const std::deque<tc::Route>& routes_;
  std::map<std::string_view, svg::Point> stop_to_coordinates_;
};

class MapRenderer {
 public:
  MapRenderer(const json::Document& doc);

 private:
  const json::Document& doc_;
};

}
