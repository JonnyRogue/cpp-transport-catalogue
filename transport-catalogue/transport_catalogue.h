#pragma once

#include "geo.h"
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
    void AddStop(std::string_view stop_name, const double lat, const double lng, const std::vector<std::pair<std::string, double>>& dst_info);
    void AddBus(const QueryInputBus& query);
    void SetDistance(std::string_view stop_from, std::string_view stop_to, size_t distance);
    const std::unordered_map<PairStop, double, DistanceHasher>& GetDistance() const;
    BusQueryInput GetBusInfo(const Bus& bus) const;
    std::optional<std::set<std::string>> GetStopInfo(std::string_view query);
    Bus* FindBus(std::string_view bus_name);
    Stop* FindStop(const std::string_view stop_name);
    const std::deque<Bus>& GetAllBuses() const;
	const std::deque<Stop>& GetAllStops() const;
    void AddBusForSerializator(std::string bus_name, RouteType type, std::vector<std::string> stop_names);
    const std::map<std::string_view, const Bus*> GetBuses() const;
    const std::map<std::string_view, const Stop*> GetStops() const;
    const std::set<std::string_view>& GetBusesForStop(std::string_view stop) const;
    double GetCalculateDistance(const Stop* first_route, const Stop* second_route);
    
private: 
    int id = 0;
    const std::set<std::string_view> empty_route{};
    std::deque<Stop> stops;
    std::deque<Bus> buses;
    std::unordered_map<std::string_view, Stop*> map_all_stops;
    std::unordered_map<std::string_view, Bus*> map_all_buses;
    std::unordered_map<PairStop, double, DistanceHasher> map_distance_to_stop;
    std::unordered_map<std::string_view, std::set<std::string_view>> stop_to_bus_map; 
    
}; //TransportCatalogue
}  //transport_catalogue
