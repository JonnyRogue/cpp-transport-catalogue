#include "request_handler.h"

namespace transport_catalogue {
namespace request {
    
RequestHandler::RequestHandler(const TransportCatalogue& db) : db_(db) {}

std::optional<BusQueryInput> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    return db_.GetBusInfo(bus_name);
}

const std::unordered_set<const Bus*> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    const std::unordered_set<const Bus*> a = db_.GetStopInfo(stop_name).buses_name;
    return a;
}
    
} //namespace request
} //namespace transport_catalogue
