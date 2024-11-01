#include <iostream>

#include "json.h"
#include "json_reader.h"
#include "request_handler.h"
#include "transport_catalogue.h"

int main() {
  tc::TransportCatalogue catalogue;
  const auto &doc = json::Load(std::cin);
  RequestHandler request_handler(catalogue);

  tc::SerializeToCatalogue(catalogue, doc);
  json::Document result = request_handler.ProcessingRequests(doc);
  auto svg_map = result.GetRoot().AsArray().at(0).AsMap().at("map").AsString();
//  svg_map.erase(std::remove_if(svg_map.begin(), svg_map.end(), [](char ch) { return ch == '\r' || ch == '\n'; }), svg_map.end());
  std::cout << svg_map << std::endl;
//  json::Print(result, std::cout);

  return 0;
}
