#pragma once

#include "transport_catalogue.h"
namespace transport_catalogue {
namespace detail {
namespace output {
    
void PrintBusInfo(std::ostream& output, const BusQueryInput& bus_info);

void PrintStopInfo(std::ostream& output, const StopQueryInput& stop_info);

std::string_view ParseRequest(std::string_view output_info);

void PrintFinalResult(const TransportCatalogue& catalogue, std::istream& input);

}//namespace output
}//namespace detail
}//namespace transport_catalogue
