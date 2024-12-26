#include "transport_catalogue.h"
#include "transport_router.h"

using namespace router;

Router::Router(Params settings, const tc::TransportCatalogue &catalogue)
    : catalogue_(catalogue),
      params_(settings) {
  const auto &stops = catalogue.GetSortedAllNonEmptyStops();
  const auto &routes = catalogue.GetSortedAllNonEmptyRoutes();
  const size_t vertex_count = stops.size() * 2;  // По две вершины на остановку
  graph_ = graph::DirectedWeightedGraph<Minutes>(vertex_count);
  vertex_idx_to_stopname_.reserve(catalogue.GetSortedAllNonEmptyStops().size() * 2);

  AddStopsToGraph(stops);
  AddRoutesToGraph(routes);
  router_ = std::make_unique<graph::Router<Minutes>>(graph_);
}

RouteInfo Router::FindRoute(std::string_view from, std::string_view to) const {
  // Если такой остановки нет, метод at выкинет исключение за нас, которое будет обработано
  const graph::VertexId vertex_from = stopname_to_vertexes_.at(from).out;
  const graph::VertexId vertex_to = stopname_to_vertexes_.at(to).out;
  const auto route = router_->BuildRoute(vertex_from, vertex_to);
  if (!route) {
    // Если не удалось построить маршрут, выкидываем исключение, которое будет обработано
    throw GraphError("Failed to build route");
  }

  RouteInfo route_info;
  route_info.total_time = route->weight;
  route_info.items.reserve(route->edges.size());

  for (const auto &edge_id : route->edges) {
    const auto &edge = graph_.GetEdge(edge_id);
    const auto &route_edge_info = edges_[edge_id];
    if (route_edge_info.has_value()) {
      route_info.items.emplace_back(RouteInfo::Moving{
          route_edge_info->routename,
          edge.weight,
          route_edge_info->steps_count,
      });
    } else {
      const graph::VertexId vertex_id = edge.from;
      route_info.items.emplace_back(RouteInfo::Waiting{
          vertex_idx_to_stopname_[vertex_id],
          edge.weight,
      });
    }
  }
  return route_info;
}

void Router::AddStopsToGraph(const std::vector<const tc::Stop *> &stops) {
  // Вершины нумеруются с 0 до stops.size() * 2 - 1
  graph::VertexId vertex_id = 0;

  for (const auto &stop : stops) {
    StopVertex &vertex = stopname_to_vertexes_[stop->name_];

    vertex.in = vertex_id++;
    vertex.out = vertex_id++;
    vertex_idx_to_stopname_[vertex.in] = stop->name_;
    vertex_idx_to_stopname_[vertex.out] = stop->name_;

    edges_.emplace_back(std::nullopt);
    graph_.AddEdge({vertex.out, vertex.in, params_.bus_wait_time});
  }
}

void Router::AddRoutesToGraph(const std::vector<const tc::Route *> &routes) {
  for (const auto &route : routes) {
    const auto &route_stops = route->stops_;
    const size_t stop_count = route_stops.size();
    if (stop_count <= 1) {
      continue;
    }
    auto compute_distance_from = [this, &route_stops](size_t stop_idx) {
      return catalogue_.GetDistance({route_stops[stop_idx], route_stops[stop_idx + 1]});
    };
    for (size_t begin_i = 0; begin_i + 1 < stop_count; ++begin_i) {
      const graph::VertexId start = stopname_to_vertexes_.at(route_stops[begin_i]->name_).in;
      size_t total_distance = 0;
      for (size_t end_i = begin_i + 1; end_i < stop_count; ++end_i) {
        total_distance += compute_distance_from(end_i - 1);
        edges_.emplace_back(RouteEdge{route->name_, end_i - begin_i});
        graph_.AddEdge({start,
                        stopname_to_vertexes_.at(route_stops[end_i]->name_).out,
                        Minutes(static_cast<double>(total_distance) / (params_.bus_velocity * 1000 / 60.0))});
      }
    }
  }
}
