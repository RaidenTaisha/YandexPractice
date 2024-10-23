#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

void ParseAndPrintStat(const tc::TransportCatalogue& catalogue, std::string_view request, std::ostream& output);
