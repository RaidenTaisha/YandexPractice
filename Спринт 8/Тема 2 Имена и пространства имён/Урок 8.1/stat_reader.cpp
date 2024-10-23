#include "stat_reader.h"

#include <iostream>

using namespace std::string_view_literals;

void ParseAndPrintStat(const TransportCatalogue &catalogue, std::string_view request, std::ostream &output) {
  if ("Bus"sv == request.substr(0, request.find_first_of(' '))) {
    size_t route_name_pos = "Bus "sv.size();
    try {
      output << request << ": " << catalogue.GetRouteInfo(request.substr(route_name_pos)).total_stops
             << " stops on route, "
             << catalogue.GetRouteInfo(request.substr(route_name_pos)).unique_stops << " unique stops, "
             << catalogue.GetRouteInfo(request.substr(route_name_pos)).route_length << " route length";
    } catch (const std::out_of_range &e) {
      output << "not found";
    }
  } else if ("Stop"sv == request.substr(0, request.find_first_of(' '))) {
    size_t stop_name_pos = "Stop "sv.size();
    try {
      output << request << ": ";
      auto routes = catalogue.GetRoutes(request.substr(stop_name_pos));
      if (routes.empty()) {
        output << "no buses";
      } else {
        output << "buses";
        for (const auto &route : routes)
          output << " " << route.name_;
      }
    } catch (const std::out_of_range &e) {
      output << "not found";
    }
  }
  output << std::endl;
}
