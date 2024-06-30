#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

enum class QueryType {
  NewBus,
  BusesForStop,
  StopsForBus,
  AllBuses,
};

struct Query {
  QueryType type;
  string bus;
  string stop;
  vector<string> stops;
};

istream& operator>>(istream& is, Query& q) {
  string typeStr;
  is >> typeStr;
  if (typeStr == "NEW_BUS"s) {
    q.type = QueryType::NewBus;
    is >> q.bus;
    int stop_count = 0;
    is >> stop_count;
    q.stops.resize(stop_count);
    for (auto& stop : q.stops) {
      is >> stop;
    }
  } else if (typeStr == "BUSES_FOR_STOP"s) {
    q.type = QueryType::BusesForStop;
    is >> q.stop;
  } else if (typeStr == "STOPS_FOR_BUS"s) {
    q.type = QueryType::StopsForBus;
    is >> q.bus;
  } else if (typeStr == "ALL_BUSES"s) {
    q.type = QueryType::AllBuses;
  }

  return is;
}

struct BusesForStopResponse {
  vector<string> buses;
};

ostream& operator<<(ostream& os, const BusesForStopResponse& r) {
  if (r.buses.empty()) {
    os << "No stop"s;
  } else {
    bool is_first = true;
    for (const string& bus : r.buses) {
      if (!is_first) {
        cout << " "s;
      }
      is_first = false;
      cout << bus;
    }
  }
  return os;
}

struct StopsForBusResponse {
  string bus;
  vector<pair<string, vector<string>>> stops_for_buses;
};

ostream& operator<<(ostream& os, const StopsForBusResponse& r) {
  if (r.stops_for_buses.empty()) {
    os << "No bus"s;
  } else {
    bool first = true;
    for (const auto& stop_and_buses : r.stops_for_buses) {
      if (!first) {
        os << endl;
      }
      first = false;
      os << "Stop "s << stop_and_buses.first << ":"s;
      if (stop_and_buses.second.size() == 1) {
        os << " no interchange"s;
      } else {
        for (const auto& bus : stop_and_buses.second) {
          if (bus != r.bus) {
            os << " "s << bus;
          }
        }
      }
    }
  }
  return os;
}

struct AllBusesResponse {
  map<string, vector<string>> buses_to_stops;
};

ostream& operator<<(ostream& os, const AllBusesResponse& r) {
  if (r.buses_to_stops.empty()) {
    os << "No buses"s;
  } else {
    bool first = true;
    for (const auto& bus_and_stops : r.buses_to_stops) {
      if (!first) {
        os << endl;
      }
      first = false;
      os << "Bus "s << bus_and_stops.first << ":"s;
      for (const auto& stop : bus_and_stops.second) {
        os << " "s << stop;
      }
    }
  }
  return os;
}

class BusManager {
 public:
  void AddBus(const string& bus, const vector<string>& stops) {
    buses_to_stops_.insert(make_pair(bus, stops));
    for (const auto& stop : stops) {
      stops_to_buses_[stop].push_back(bus);
    }
  }

  BusesForStopResponse GetBusesForStop(const string& stop) const {
    if (stops_to_buses_.count(stop) == 0) {
      return BusesForStopResponse{vector<string>()};
    } else {
      return BusesForStopResponse{stops_to_buses_.at(stop)};
    }
  }

  StopsForBusResponse GetStopsForBus(const string& bus) const {
    vector<pair<string, vector<string>>> result;

    if (buses_to_stops_.count(bus) > 0) {
      for (const auto& stop : buses_to_stops_.at(bus)) {
        result.push_back(make_pair(stop, stops_to_buses_.at(stop)));
      }
    }
    return StopsForBusResponse{bus, result};
  }

  AllBusesResponse GetAllBuses() const {
    return AllBusesResponse{buses_to_stops_};
  }

 private:
  map<string, vector<string>> buses_to_stops_;
  map<string, vector<string>> stops_to_buses_;
};

int main() {
  int query_count;
  Query q;

  cin >> query_count;

  BusManager bm;
  for (int i = 0; i < query_count; ++i) {
    cin >> q;
    switch (q.type) {
      case QueryType::NewBus:
        bm.AddBus(q.bus, q.stops);
        break;
      case QueryType::BusesForStop:
        cout << bm.GetBusesForStop(q.stop) << endl;
        break;
      case QueryType::StopsForBus:
        cout << bm.GetStopsForBus(q.bus) << endl;
        break;
      case QueryType::AllBuses:
        cout << bm.GetAllBuses() << endl;
        break;
    }
  }
}
