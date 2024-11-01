#pragma once

#include "json.h"
#include "transport_catalogue.h"

namespace tc {

void SerializeToCatalogue(TransportCatalogue& catalogue, json::Document& doc);

json::Document ProcessingRequests(TransportCatalogue& catalogue, json::Document& doc);

}
