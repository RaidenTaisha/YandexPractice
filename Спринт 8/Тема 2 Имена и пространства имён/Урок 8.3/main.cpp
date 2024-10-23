#include <iostream>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;
using namespace tc;

int main() {
  TransportCatalogue catalogue;

  {
    InputReader reader;
    reader.ReadCommands(cin);
    reader.ApplyCommands(catalogue);
  }

  ParseCommands(catalogue, cin);
}
