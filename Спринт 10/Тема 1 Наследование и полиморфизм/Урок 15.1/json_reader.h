#pragma once

#include "json.h"
#include "transport_catalogue.h"

namespace tc {

void SerializeToCatalogue(TransportCatalogue& catalogue, const json::Document& doc);

}
