#pragma once
#include "domain.h"

namespace transport_catalogue {

struct DistanceHasher {
    std::size_t operator()(const PairStop pair_stops) const noexcept {
        return hasher_(static_cast<const void*>(pair_stops.first)) * 37 + hasher_(static_cast<const void*>(pair_stops.second));
    }
private:
    std::hash<const void*> hasher_;
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
    const std::unordered_set<const Bus*> GetAllBuses() const;
	const std::unordered_set<const Stop*> GetAllStops() const;
    
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
    std::unordered_map<const Stop*, std::unordered_set<const Bus*>> stop_to_bus_map; 
    
}; //TransportCatalogue
}  //transport_catalogue
