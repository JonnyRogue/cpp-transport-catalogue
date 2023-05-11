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
#include <iomanip>
#include <string_view>
#include <unordered_map>
#include <unordered_set>


#include "geo.h"

namespace transport_catalogue {

enum class RouteType { CIRCLE, TWO_DIRECTIONAL };

struct Bus {
    std::string name_bus;
    RouteType type;
    std::vector<std::string_view> stop_names;
    std::set<std::string_view> unique_stops;
    size_t GetStopsCount() const;
};

struct Stop {
    std::string name;
    geo::Coordinates coord;
};

using PairStop = std::pair<const Stop*, const Stop*>;

struct DistanceHasher {
    std::size_t operator()(const PairStop pair_stops) const noexcept {
        return hasher_(static_cast<const void*>(pair_stops.first)) * 37 + hasher_(static_cast<const void*>(pair_stops.second));
    }
private:
    std::hash<const void*> hasher_;
};

struct BusQueryInput {
    std::string_view number;
    size_t stops_count = 0;
    size_t unique_stops_count = 0;
    int rout_length = 0;
    double curvature = 0.0;
};

std::ostream& operator<<(std::ostream& output, const BusQueryInput& bus_info);

class TransportCatalogue {

public:  
    void AddStop(Stop stop);
    void AddBus(Bus bus);
    void AddDistance(std::string_view stop_from, std::string_view stop_to, int distance);
    std::optional<BusQueryInput> GetBusInfo(std::string_view bus) const;
    const std::set<std::string_view>* GetBusesOnTheStop(std::string_view stop_name) const;
    
private:
    int CalculateRouteLength(const Bus* bus_info) const;
    double CalculateGeographicLength(const Bus* bus_info) const;

private: 
    std::deque<Stop> stops;
    std::deque<Bus> buses;
    std::unordered_map<std::string_view, const Stop*> map_all_stops;
    std::unordered_map<std::string_view, const Bus*> map_all_buses;
    std::unordered_map<PairStop, int, DistanceHasher> map_distance_to_stop;
    std::unordered_map<std::string_view, std::set<std::string_view>> stop_to_bus_map;
    
}; //TransportCatalogue
}  //transport_catalogue
