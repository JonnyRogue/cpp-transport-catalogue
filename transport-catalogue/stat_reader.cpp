#include "stat_reader.h"

namespace transport_catalogue { 
namespace detail {
namespace output{

using namespace std::literals;

std::string_view ParseBusStatisticsRequest(std::string_view output_info) {
    size_t bus_begin = output_info.find(" "sv) + (" "sv).size();
    return output_info.substr(bus_begin, output_info.size() - bus_begin);
}

std::string_view ParseBusesPassingThroughStopRequest(std::string_view output_info) {
    size_t stop_begin = output_info.find(" "sv) + (" "sv).size();
    return output_info.substr(stop_begin, output_info.size() - stop_begin);
}

void PrintNotFoundBusesAndStops(std::ostream& output, std::string_view stop_name, const std::set<std::string_view>* buses) {
    if (!buses) {
        output << "Stop " << stop_name << ": not found" << std::endl;
    } else if (buses->empty()) {
        output << "Stop " << stop_name << ": no buses" << std::endl;
    } else {
        output << "Stop " << stop_name << ": buses ";
        size_t index = 0;
        for (std::string_view bus : *buses) {
            if (index++ != 0)
                output << " ";
            output << bus;
        }
        output << std::endl;
    }
}
}//namespace output
}//namespace detail
}//namespace transport_catalogue 
