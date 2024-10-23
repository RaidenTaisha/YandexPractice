#pragma once

#include <iostream>
#include <string_view>

#include "transport_catalogue.h"

void ParseAndPrintStat(const tc::TransportCatalogue& catalogue, std::string_view request, std::ostream& output);

void ParseCommands(const tc::TransportCatalogue& catalogue, std::istream& is, size_t count);
