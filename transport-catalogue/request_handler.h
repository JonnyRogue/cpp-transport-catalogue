#pragma once
#include "transport_catalogue.h"

namespace transport_catalogue {
namespace request {

class RequestHandler {
public:
    RequestHandler(const TransportCatalogue& db);

    std::optional<BusQueryInput> GetBusStat(const std::string_view& bus_name) const;

    const std::unordered_set<const Bus* > GetBusesByStop(const std::string_view& stop_name) const;

private:
    const TransportCatalogue& db_;
};

} //namespace request
} //namespace transport_catalogue
