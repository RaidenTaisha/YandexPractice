#include <iostream>

#include "json.h"
#include "json_reader.h"
#include "transport_catalogue.h"

int main() {
  tc::TransportCatalogue catalogue;

  auto doc = json::Load(std::cin);

  tc::SerializeToCatalogue(catalogue, doc);
  auto res = tc::ProcessingRequests(catalogue, doc);

  json::Print(res, std::cout);

  return 0;
}
