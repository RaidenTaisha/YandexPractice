#include "stat_reader.h"

#include <string>
#include <stdexcept>

using namespace tc;

void PrintBusStat(const TransportCatalogue &catalogue, const std::string_view id, std::ostream &output) {
  const auto bus = catalogue.GetRoute(id);
  if (nullptr == bus) {
    output << "Bus " << id << ": not found" << std::endl;
  } else {
    /* Я считаю что тут не нужен блок try {} catch () {}, потому что непонятно какой результат возвращать пользователю
       в случе если такой остановки нет. Я трактую такой вариант как возникновение исключетельной ситуации, и считаю
       справедливым выход программы из строя. Ибо нефиг кормить программу абсурдными данными */
//      try {
    const auto info = catalogue.GetRouteInfo(id);
    output << "Bus " << id << ": "
           << info.total_stops_ << " stops on route, "
           << info.unique_stops_ << " unique stops, "
           << static_cast<double>(info.real_length_) << " route length, "
           << static_cast<double>(info.real_length_) / info.direct_length_ << " curvature" << std::endl;
//      } catch (const std::out_of_range &e) {}
  }
}

void PrintStopStat(const TransportCatalogue &catalogue, const std::string_view id, std::ostream &output) {
  const auto *stop = catalogue.GetStop(id);
  if (nullptr == stop) {
    output << "Stop " << id << ": not found";
  } else {
    try {
      const auto &names = catalogue.GetRoutes(id);
      output << "Stop " << id << ": buses";
      for (const auto &bus : names)
        output << " " << bus;
    } catch (const std::out_of_range &) {
      output << "Stop " << id << ": no buses";
    }
  }
  output << std::endl;
}

void ParseAndPrintStat(const TransportCatalogue &catalogue, std::string_view request, std::ostream &output) {
  size_t command_pos = request.find_first_of(' ');
  const std::string_view command = request.substr(0, command_pos);
  const std::string_view id = request.substr(command_pos + 1);

  if ("Bus" == command)
    PrintBusStat(catalogue, id, output);
  else if ("Stop" == command)
    PrintStopStat(catalogue, id, output);
}

void ParseCommands(const tc::TransportCatalogue &catalogue, std::istream &is) {
  size_t count;
  is >> count >> std::ws;
  for (size_t i = 0; i < count; ++i) {
    std::string line;
    getline(is, line);
    ParseAndPrintStat(catalogue, line, std::cout);
  }
}
