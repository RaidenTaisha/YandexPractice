#include "geo.h"
#include "input_reader.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <string>
#include <istream>

/**
* Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
*/
Coordinates ParseCoordinates(std::string_view str) {
  static const double nan = std::nan("");

  auto not_space = str.find_first_not_of(' ');
  auto comma = str.find(',');

  if (comma == std::string::npos) {
    return {nan, nan};
  }

  auto not_space2 = str.find_first_not_of(' ', comma + 1);

  double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
  double lng = std::stod(std::string(str.substr(not_space2)));

  return {lat, lng};
}

/**
* Удаляет пробелы в начале и конце строки
*/
std::string_view Trim(std::string_view string) {
  const auto start = string.find_first_not_of(' ');
  if (start == std::string::npos) {
    return {};
  }
  return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
* Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
*/
std::vector<std::string_view> Split(std::string_view string, char delim) {
  std::vector<std::string_view> result;

  size_t pos = 0;
  while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
    auto delim_pos = string.find(delim, pos);
    if (delim_pos == std::string::npos) {
      delim_pos = string.size();
    }
    if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
      result.push_back(substr);
    }
    pos = delim_pos + 1;
  }

  return result;
}

/**
* Парсит строку вида "3900m to Marushkino, 4200m to Tolstopaltsevo..." и возвращает вектор пар
* ((остановка А, остановка Б), расстояние)
*/
std::vector<std::pair<size_t, std::string>> ParseDistances(const std::string &data) {
  using namespace std::string_view_literals;
  auto delim = "m to "sv;
  std::vector<std::pair<size_t, std::string>> distances;
  auto parts = Split(data, ',');

  for (auto &part : parts) {
    auto delim_pos = part.find(delim);

    if (delim_pos == std::string::npos)
      continue;

    auto distance = std::stoul(std::string(part.substr(0, delim_pos)));
    auto dest = part.substr(delim_pos + delim.size());
    distances.emplace_back(distance, dest);
  }

  return distances;
}

/**
* Парсит маршрут.
* Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
* Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
*/
std::vector<std::string_view> ParseRoute(std::string_view route) {
  if (route.find('>') != std::string::npos) {
    return Split(route, '>');
  }

  auto stops = Split(route, '-');
  std::vector<std::string_view> results(stops.begin(), stops.end());
  results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

  return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
  const auto colon_pos = line.find(':');
  if (colon_pos == std::string::npos) {
    return {};
  }

  const auto space_pos = line.find(' ');
  if (space_pos >= colon_pos) {
    return {};
  }

  const auto not_space = line.find_first_not_of(' ', space_pos);
  if (not_space >= colon_pos) {
    return {};
  }

  return {std::string(line.substr(0, space_pos)),
          std::string(line.substr(not_space, colon_pos - not_space)),
          std::string(line.substr(colon_pos + 1))};
}

void InputReader::ParseLine(std::string_view line) {
  auto command_description = ParseCommandDescription(line);
  if (command_description)
    commands_.push_back(std::move(command_description));
}

void InputReader::ApplyCommands(tc::TransportCatalogue &catalogue) const {
  std::vector<CommandDescription> sorted_commands(commands_.begin(), commands_.end());
  std::sort(sorted_commands.begin(),
            sorted_commands.end(),
            [](const CommandDescription &lhs, const CommandDescription &rhs) {
              return lhs.command > rhs.command;
            });

  for (const auto &command : sorted_commands)
    if (command.command == "Stop")
      catalogue.AddStop(command.id, ParseCoordinates(command.description));
    else if (command.command == "Bus")
      /* Я считаю что тут не нужен блок try {} catch () {}, потому что непонятно какой результат возвращать пользователю
         в случе если метод AddRoute кинет исключение. Я трактую такой вариант как возникновение исключетельной
         ситуации, и считаю справедливым выход программы из строя. */
//      try {
      catalogue.AddRoute(command.id, ParseRoute(command.description));
//      } catch (const std::out_of_range& e) {}

  for (const auto &command : sorted_commands)
    if (command.command == "Stop")
      for (const auto &dist : ParseDistances(command.description))
        catalogue.SetDistance({catalogue.GetStop(command.id), catalogue.GetStop(dist.second)}, dist.first);
}

void InputReader::ReadCommands(std::istream &is) {
  size_t count;
  is >> count >> std::ws;
  for (size_t i = 0; i < count; ++i) {
    std::string line;
    getline(is, line);
    ParseLine(line);
  }
}
