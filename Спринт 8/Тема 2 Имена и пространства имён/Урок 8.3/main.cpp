#include <iostream>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;
using namespace tc;

int main() {
  TransportCatalogue catalogue;

  int base_request_count;
  cin >> base_request_count >> ws;

  {
    InputReader reader;
    reader.ReadCommands(cin, base_request_count);
    reader.ApplyCommands(catalogue);
  }

  int stat_request_count;
  cin >> stat_request_count >> ws;
  ParseCommands(catalogue, cin, stat_request_count);
}
