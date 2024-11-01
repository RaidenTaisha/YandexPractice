#include "map_renderer.h"

using namespace renderer;

void MapRenderer::GetRouteLines(const std::vector<const tc::Route *> &routes,
                                const SphereProjector &sp,
                                svg::Document &result) const {
  size_t color_count = 0;
  for (const auto &route : routes) {
    if (route->stops_.empty()) {
      continue;
    }

    svg::Polyline line;
    for (const auto &stop : route->stops_) {
      line.AddPoint(sp(stop->coordinates_));
    }

    line.SetStrokeColor(params_.color_palette_.at(color_count));
    line.SetFillColor("none");
    line.SetStrokeWidth(params_.line_width_);
    line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    result.Add(line);

    if (color_count < (params_.color_palette_.size() - 1)) {
      ++color_count;
    } else {
      color_count = 0;
    }
  }
}

void MapRenderer::GetRouteLabel(const std::vector<const tc::Route *> &routes,
                                const SphereProjector &sp,
                                svg::Document &result) const {
  size_t color_count = 0;
  for (const auto &route : routes) {
    if (route->stops_.empty()) {
      continue;
    }

    svg::Text text;
    text.SetFillColor(params_.color_palette_.at(color_count));
    text.SetPosition(sp(route->stops_.at(0)->coordinates_));
    text.SetOffset({params_.route_label_offset_.first,
                    params_.route_label_offset_.second});

    text.SetFontSize(params_.route_label_font_size_);
    text.SetFontFamily("Verdana").SetFontWeight("bold");
    text.SetData(route->name_);

    if (color_count < (params_.color_palette_.size() - 1)) {
      ++color_count;
    } else {
      color_count = 0;
    }

    svg::Text underlayer;
    underlayer.SetFillColor(params_.underlayer_color_);
    underlayer.SetStrokeColor(params_.underlayer_color_);
    underlayer.SetStrokeWidth(params_.underlayer_width_);
    underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    underlayer.SetPosition(sp(route->stops_.at(0)->coordinates_));
    underlayer.SetOffset({params_.route_label_offset_.first,
                          params_.route_label_offset_.second});

    underlayer.SetFontSize(params_.route_label_font_size_);
    underlayer.SetFontFamily("Verdana").SetFontWeight("bold");
    underlayer.SetData(route->name_);

    result.Add(underlayer);
    result.Add(text);

    {
      size_t end_stop_index = route->stops_.size() / 2;
      bool check_second_text = !route->is_rounded && (route->stops_.at(end_stop_index) != route->stops_.at(0));
      if (check_second_text) {
        svg::Text text2{text};
        svg::Text underlayer2{underlayer};
        text2.SetPosition(sp(route->stops_.at(end_stop_index)->coordinates_));
        underlayer2.SetPosition(sp(route->stops_.at(end_stop_index)->coordinates_));

        result.Add(underlayer2);
        result.Add(text2);
      }
    }
  }
}

void MapRenderer::GetStopsPoints(const std::vector<const tc::Stop *> &stops,
                                 const SphereProjector &sp,
                                 svg::Document &result) const {
  for (const auto &stop : stops) {
    svg::Circle symbol;
    symbol.SetCenter(sp(stop->coordinates_));
    symbol.SetRadius(params_.stop_radius_);
    symbol.SetFillColor("white");
    result.Add(symbol);
  }
}

void MapRenderer::GetStopsLabels(const std::vector<const tc::Stop *> &stops,
                                 const SphereProjector &sp,
                                 svg::Document &result) const {
  for (const auto &stop : stops) {
    svg::Text text;
    svg::Text underlayer;
    text.SetFillColor("black");
    text.SetPosition(sp(stop->coordinates_));
    text.SetOffset({params_.stop_label_offset_.first,
                    params_.stop_label_offset_.second});
    text.SetFontSize(params_.stop_label_font_size_).SetFontFamily("Verdana");
    text.SetData(stop->name_);

    underlayer.SetFillColor(params_.underlayer_color_);
    underlayer.SetStrokeColor(params_.underlayer_color_);
    underlayer.SetStrokeWidth(params_.underlayer_width_);
    underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    underlayer.SetPosition(sp(stop->coordinates_));
    underlayer.SetOffset({params_.stop_label_offset_.first,
                          params_.stop_label_offset_.second});
    underlayer.SetFontSize(params_.stop_label_font_size_).SetFontFamily("Verdana");
    underlayer.SetData(stop->name_);

    result.Add(underlayer);
    result.Add(text);
  }
}

svg::Document MapRenderer::RenderSVG(const std::vector<const tc::Route *>& routes,
                                     const std::vector<const tc::Stop *>& stops) const {
  svg::Document result;
  std::vector<geo::Coordinates> route_stops_coord;

  for (const auto &stop : stops) {
    route_stops_coord.push_back(stop->coordinates_);
  }
  SphereProjector sp(route_stops_coord.begin(), route_stops_coord.end(),
                     params_.width_, params_.height_,
                     params_.padding_);

  GetRouteLines(routes, sp, result);
  GetRouteLabel(routes, sp, result);
  GetStopsPoints(stops, sp, result);
  GetStopsLabels(stops, sp, result);

  return result;
}
