#pragma once

#include "transport_catalogue.h"
namespace transport_catalogue {
namespace detail {
namespace output {

std::string_view ParseBusStatisticsRequest(std::string_view output_info);

std::string_view ParseBusesPassingThroughStopRequest(std::string_view output_info);

void PrintNotFoundBusesAndStops(std::ostream& output, std::string_view stop_name, const std::set<std::string_view>* buses);

}//namespace output
}//namespace detail
}//namespace transport_catalogue 
