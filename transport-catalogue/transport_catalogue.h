#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <regex>
#include <deque>
#include <list>
#include <execution>
#include <numeric>
#include <optional>
#include <set>
#include <functional>
#include <iomanip>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "geo.h"

namespace transport_catalogue {
    
enum class RouteType { 
    CIRCLE, 
    TWO_DIRECTIONAL
};
    
struct Stop {
    std::string name;
    geo::Coordinates coord;
};

using PairStop = std::pair<const Stop*, const Stop*>;
using DeqStop = std::deque<const Stop*>;
using StringVec = std::vector<std::string>;
    
struct Bus {
    RouteType type;
    std::string name_bus;
    DeqStop stop_names;
};

struct DistanceHasher {
    std::size_t operator()(const PairStop pair_stops) const noexcept {
        return hasher_(static_cast<const void*>(pair_stops.first)) * 37 + hasher_(static_cast<const void*>(pair_stops.second));
    }
private:
    std::hash<const void*> hasher_;
};

struct BusQueryInput {
    bool bus_not_found_ = false;
    std::string number;
    size_t stops_count = 0;
    size_t unique_stops_count = 0;
    double route_length = 0.0;
    double curvature = 0.0;
};

struct StopQueryInput {
    bool not_found_ = false;
    std::string name;
    std::set<std::string_view> buses_name;
};
    
class TransportCatalogue {

public:  
    void AddStop(const Stop& stop);
    void AddBus(const Bus& bus);
    void SetDistance(std::string_view stop_from, std::string_view stop_to, size_t distance);
    BusQueryInput GetBusInfo(std::string_view bus) const;
    StopQueryInput GetStopInfo(std::string_view stop_name) const;
    const Bus* FindBus(const std::string_view bus_name) const;
    const Stop* FindStop(const std::string_view stop_name) const;
    size_t GetDistance(std::string_view stop_from, std::string_view stop_to) const;
    
private:
    double CalculateRouteLength(std::string_view bus_info) const;
    double CalculateGeographicLength(std::string_view bus_info) const;
    size_t CalculateUniqueStops(std::string_view bus) const;

private: 
    std::deque<Stop> stops;
    std::deque<Bus> buses;
    std::unordered_map<std::string_view, const Stop*> map_all_stops;
    std::unordered_map<std::string_view, const Bus*> map_all_buses;
    std::unordered_map<PairStop, int, DistanceHasher> map_distance_to_stop;
    std::unordered_map<const Stop*, std::set<std::string_view>> stop_to_bus_map; // проще было так реализовать, по - моему... 
    
}; //TransportCatalogue
}  //transport_catalogue
