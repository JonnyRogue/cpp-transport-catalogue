#include "input_reader.h"
#include "stat_reader.h"

namespace transport_catalogue {
namespace detail {
namespace input {

using namespace std::literals;
using namespace transport_catalogue::detail::output;

std::vector<std::pair<std::string_view, int>> ParseDistancesToStops(std::string_view input_info) {
    std::vector<std::pair<std::string_view, int>> result;
    size_t start = input_info.find(',');
    start = input_info.find(',', start + 1) + (" "sv).size();
    size_t end = start;
    while (start != std::string_view::npos) {
        end = input_info.find("m"sv, start);
        int distance = std::stoi(std::string(input_info.substr(start, end - start)));
        start = end + ("m to "sv).size();
        end = input_info.find(","sv, start);
        std::string_view stop_to = input_info.substr(start, end - start);
        result.emplace_back(stop_to, distance);
        start = (end == std::string_view::npos)  ?  end  :  end + (" "sv).size();
    }
    return result;
}

std::pair<transport_catalogue::Stop, bool> ParseStopsAndCoord(const std::string& input_info) {
    transport_catalogue::Stop stop;
    size_t stop_begin = ("Stop "s).size();
    size_t stop_end = input_info.find(": "s, stop_begin);
    stop.name = input_info.substr(stop_begin, stop_end - stop_begin);
    size_t lat_begin = stop_end + (": "s).size();
    size_t lat_end = input_info.find(","s, lat_begin);
    stop.coord.lat = std::stod(input_info.substr(lat_begin, lat_end - lat_begin));
    size_t lng_begin = lat_end + (", "s).size();
    size_t lng_end = input_info.find(","s, lng_begin);
    stop.coord.lng = std::stod(input_info.substr(lng_begin, lng_end - lng_begin));
    bool has_stops_info = lng_end != std::string_view::npos;
    return {std::move(stop), has_stops_info};
}

transport_catalogue::Bus ParseRouteForBus(std::string_view input_info) {
    transport_catalogue::Bus bus_;
    size_t bus_start = input_info.find(' ') + (" "sv).size();
    size_t bus_end = input_info.find(": "sv, bus_start);
    bus_.name_bus = input_info.substr(bus_start, bus_end - bus_start);
    bus_.type = (input_info[input_info.find_first_of("->")] == '>') ? transport_catalogue::RouteType::CIRCLE :transport_catalogue::RouteType::TWO_DIRECTIONAL;
    std::string_view stop_type = (bus_.type == transport_catalogue::RouteType::CIRCLE) ? " > "sv : " - "sv;
    size_t stop_begin = bus_end + (": "sv).size();
    while (stop_begin <= input_info.length()) {
        size_t stop_end = input_info.find(stop_type, stop_begin);
        bus_.stop_names.push_back(input_info.substr(stop_begin, stop_end - stop_begin));
        stop_begin = (stop_end == std::string_view::npos) ? stop_end : stop_end + stop_type.size();
    }
    bus_.unique_stops = {bus_.stop_names.begin(), bus_.stop_names.end()};
    return bus_;
}

void InputAndOutputInformation(std::istream& input) {
    TransportCatalogue catalogue;
    int queries = 0;
    input >> queries;
    input.get();
    std::vector<std::string> bus_queries;
    bus_queries.reserve(queries);
    std::vector<std::pair<std::string, std::string>> stop_distances;
    stop_distances.reserve(queries);
    std::string query;
    for (int id = 0; id < queries; ++id) {
        std::getline(input, query);
        if (query.substr(0, 4) == "Stop"s) {
            auto [stop, query_] = ParseStopsAndCoord(query);
            if (query_) {
                stop_distances.emplace_back(stop.name, std::move(query));
            }
            catalogue.AddStop(std::move(stop));
        } else if (query.substr(0, 3) == "Bus"s) {
            bus_queries.emplace_back(std::move(query));
        }
    }
    for (const auto& [stop_from, query] : stop_distances) {
        for (auto [stop_to, distance] : ParseDistancesToStops(query))
            catalogue.AddDistance(stop_from, stop_to, distance);
    }
    for (const auto& bus_query : bus_queries) {
        catalogue.AddBus(ParseRouteForBus(bus_query));
    }
    input >> queries;
    input.get();
    for (int id = 0; id < queries; ++id) {
        std::getline(input, query);
        if (query.substr(0, 3) == "Bus"s) {
            std::string_view bus_number = ParseBusStatisticsRequest(query);
            if (auto bus_statistics = catalogue.GetBusInfo(bus_number)) {
                std::cout << *bus_statistics << std::endl;
            } else {
                std::cout << "Bus " << bus_number << ": not found" << std::endl;
            }
        } else if (query.substr(0, 4) == "Stop"s) {
            std::string_view stop_name = ParseBusesPassingThroughStopRequest(query);
            auto* buses = catalogue.GetBusesOnTheStop(stop_name);
            PrintNotFoundBusesAndStops(std::cout, stop_name, buses);
        }
    }
}

}//namespace input
}//namespace detail
}//namespace transport_catalogue 
