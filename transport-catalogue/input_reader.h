#pragma once

#include "transport_catalogue.h"
namespace transport_catalogue {
namespace detail {
namespace input {

std::vector<std::pair<std::string_view, int>> ParseDistancesToStops(std::string_view input_info);

std::pair<transport_catalogue::Stop, bool> ParseStopsAndCoord(const std::string& input_info);

transport_catalogue::Bus ParseRouteForBus(std::string_view input_info);

void InputAndOutputInformation(std::istream& input_info);

} //namespace input
} //namespace detail
} //namespace transport_catalogue
