#include "transport_catalogue.h"



namespace transport_catalogue {

std::ostream& operator<<(std::ostream& output, const BusQueryInput& bus_info) {
    output << "Bus " << bus_info.number << ": " << bus_info.stops_count << " stops on route, "
    << bus_info.unique_stops_count << " unique stops, ";
    output << bus_info.rout_length << " route length, ";
    output << std::setprecision(6) << bus_info.curvature << " curvature";
    return output;
}

size_t Bus::GetStopsCount() const {
    return (type == RouteType::CIRCLE) ? stop_names.size() : 2 * stop_names.size() - 1;
}

void TransportCatalogue::AddBus(Bus bus) {
    for (auto& stop : bus.stop_names) {
        stop = map_all_stops.find(stop)->first; 
    }
    bus.unique_stops = {bus.stop_names.begin(), bus.stop_names.end()};
    const auto position = buses.insert(buses.begin(), std::move(bus));
    map_all_buses.insert({position->name_bus, &(*position)});
    for (std::string_view stop : position->stop_names) {
        stop_to_bus_map[stop].insert(position->name_bus);
    }
}

void TransportCatalogue::AddStop(Stop stop) {
    const auto position = stops.insert(stops.begin(), std::move(stop));
    map_all_stops.insert({position->name, &(*position)});
    stop_to_bus_map.insert({position->name, {}});
}

void TransportCatalogue::AddDistance(std::string_view stop_from, std::string_view stop_to, int distance) {
    map_distance_to_stop.insert({{map_all_stops.at(stop_from), map_all_stops.at(stop_to)}, distance});
}

std::optional<BusQueryInput> TransportCatalogue::GetBusInfo(std::string_view bus_name) const {
    if (map_all_buses.count(bus_name) == 0) {
        return std::nullopt;
    }
    const Bus* bus_info = map_all_buses.at(bus_name);
    BusQueryInput result;
    result.number = bus_info->name_bus;
    result.stops_count = bus_info->GetStopsCount();
    result.unique_stops_count = bus_info->unique_stops.size();
    result.rout_length = CalculateRouteLength(bus_info);
    result.curvature = static_cast<double>(result.rout_length) / CalculateGeographicLength(bus_info);
    return result;
}

int TransportCatalogue::CalculateRouteLength(const Bus* bus_info) const {
    auto get_length = [this](std::string_view from, std::string_view to) {
        auto key = std::make_pair(map_all_stops.at(from), map_all_stops.at(to));
        return (map_distance_to_stop.count(key) > 0) ? map_distance_to_stop.at(key) : map_distance_to_stop.at({map_all_stops.at(to), map_all_stops.at(from)});
    };
    int one_way_route = std::transform_reduce(bus_info->stop_names.begin(), 
                                              std::prev(bus_info->stop_names.end()),
                                              std::next(bus_info->stop_names.begin()), 
                                              0, 
                                              std::plus<>(), 
                                              get_length);
    if (bus_info->type == RouteType::CIRCLE) {
        return one_way_route;
    }
    int two_way_route = std::transform_reduce(bus_info->stop_names.rbegin(), 
                                              std::prev(bus_info->stop_names.rend()), 
                                              std::next(bus_info->stop_names.rbegin()), 
                                              0, 
                                              std::plus<>(), 
                                              get_length);
    return one_way_route + two_way_route;
}

double TransportCatalogue::CalculateGeographicLength(const Bus* bus_info) const {
    double geographic_length = std::transform_reduce (std::next(bus_info->stop_names.begin()),
                                                      bus_info->stop_names.end(),
                                                      bus_info->stop_names.begin(),
                                                      0.0,
                                                      std::plus<>(), [this](std::string_view from, std::string_view to) {
                                                          return ComputeDistance(map_all_stops.at(from)->coord, map_all_stops.at(to)->coord);
                                                      });

    return (bus_info->type == RouteType::CIRCLE) ? geographic_length : geographic_length * 2.;
}

const std::set<std::string_view>* TransportCatalogue::GetBusesOnTheStop(std::string_view stop_name) const {
    if (const auto position = stop_to_bus_map.find(stop_name); position != stop_to_bus_map.end())
        return &position->second;
    return nullptr;
}

}  // namespace transport_catalogue;
