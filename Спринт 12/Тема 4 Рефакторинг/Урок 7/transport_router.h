#pragma once

#include "domain.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <chrono>
#include <memory>
#include <optional>
#include <unordered_map>
#include <variant>
#include <vector>

namespace router {

class GraphError : public std::runtime_error {
 public:
  using runtime_error::runtime_error;
};

using Minutes = std::chrono::duration<double, std::chrono::minutes::period>;

struct Params {
  Minutes bus_wait_time;
  double bus_velocity{};
};

struct RouteInfo {
  struct Waiting {
    std::string_view stopname;
    Minutes time;
  };

  struct Moving {
    std::string_view routename;
    Minutes time;
    size_t steps_count{};
  };

  using Item = std::variant<Moving, Waiting>;
  std::vector<Item> items;
  Minutes total_time;
};

class Router {
 public:
  Router(Params params, const tc::TransportCatalogue &catalogue);
  RouteInfo FindRoute(std::string_view from, std::string_view to) const;

 private:
  struct StopVertex {
    graph::VertexId in;
    graph::VertexId out;
  };

  struct RouteEdge {
    std::string_view routename;
    size_t steps_count;
  };

  void AddStopsToGraph(const std::vector<const tc::Stop *> &stops);
  void AddRoutesToGraph(const std::vector<const tc::Route *> &routes);

  const tc::TransportCatalogue &catalogue_;
  graph::DirectedWeightedGraph<Minutes> graph_;
  std::unique_ptr<graph::Router<Minutes>> router_;
  Params params_;
  std::unordered_map<std::string_view, StopVertex> stopname_to_vertexes_;
  std::vector<std::string_view> vertex_idx_to_stopname_;
  std::vector<std::optional<RouteEdge>> edges_;
};

}  // namespace tc::router
