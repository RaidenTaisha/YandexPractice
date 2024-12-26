#include "request_handler.h"

RouteInfo RequestHandler::GetRouteInfo(std::string_view name) const {
  return db_.GetRouteInfo(name);
}

const std::set<std::string_view> &RequestHandler::GetRoutes(const std::string_view stop_name) const {
  return db_.GetRoutes(stop_name);
}

svg::Document RequestHandler::RenderMap() const {
  return renderer_.RenderSVG(db_.GetSortedAllNonEmptyRoutes(), db_.GetSortedAllNonEmptyStops());
}

router::RouteInfo RequestHandler::FindRoute(std::string_view from, std::string_view to) const {
    return router_.FindRoute(from, to);
}
