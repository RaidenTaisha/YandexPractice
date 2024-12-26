#include <iostream>

#include "json_reader.h"

using namespace std;

using namespace tc;

int main() {
  TransportCatalogue catalogue;
  JsonReader reader;

  reader.ParseStream(std::cin);
  reader.FillCatalogue(catalogue);
  const renderer::Params &render_settings = reader.FillRenderSettings();
  const router::Params &router_settings = reader.FillRouterSettings();

  renderer::MapRenderer map_renderer(render_settings);
  router::Router transport_router(router_settings, catalogue);
  RequestHandler handler(catalogue, map_renderer, transport_router);
  reader.ParseRequests(handler, std::cout);

  return 0;
}