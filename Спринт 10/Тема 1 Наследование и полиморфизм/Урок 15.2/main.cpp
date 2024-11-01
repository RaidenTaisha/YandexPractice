#include <iostream>

#include "json_reader.h"

using namespace std;

using namespace tc;
using namespace renderer;

int main() {
  TransportCatalogue catalogue;
  JsonReader reader;

  try {
    reader.ParseStream(std::cin);
    reader.FillCatalogue(catalogue);
    const Params &render_settings = reader.FillRenderSettings();

    MapRenderer map_renderer(render_settings);
    RequestHandler handler(catalogue, map_renderer);
    reader.ParseRequests(handler, std::cout);
  }
  catch (const std::logic_error &) {
    return 0;
  }
}