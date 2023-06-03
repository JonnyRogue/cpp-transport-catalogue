#pragma once

#include "transport_catalogue.h"
namespace transport_catalogue {
namespace detail {
namespace input {
    
std::string ReadLine(std::istream& input);

std::vector<std::string> ReadInputData(std::istream& input);

std::pair<std::string_view, std::string_view> SplitFunc(std::string_view line, char sign);
    
std::pair<std::vector<Stop>, std::unordered_map<std::string, std::string>> SplitQuery( const StringVec& queries);
    
Bus ParseRoute(const TransportCatalogue& catalogue, std::string_view route, char sign);

std::unordered_map<std::string, size_t> ParseLine(std::string_view line);
    
DeqStop SplitStopsInRoute(const TransportCatalogue& catalogue, std::string_view& stops, char sign);

Bus InfoRoute(const TransportCatalogue& catalogue, const std::string& query_bus);
    
std::vector<Bus> SplitBuses(const TransportCatalogue& catalogue, const StringVec& queries);
    
void InputInformation(TransportCatalogue& catalogue, std::istream& input);

} //namespace input
} //namespace detail
} //namespace transport_catalogue
