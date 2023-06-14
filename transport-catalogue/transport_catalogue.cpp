#include "transport_catalogue.h"

namespace transport_catalogue {

void TransportCatalogue::AddBus(const Bus& bus) {
    buses.push_back(bus);
	const Bus* bus_ = &buses[buses.size() - 1];
	map_all_buses.insert({ bus_->name_bus, bus_ });
    for (const Stop* stop_ : bus.stop_names) {
		if (stop_to_bus_map.count(stop_)) {
			stop_to_bus_map.at(stop_).insert(bus_);
		} else {
			stop_to_bus_map[stop_].insert(bus_);
		}
	}
}

void TransportCatalogue::AddStop(const Stop& stop) {
    stops.push_back(stop);
	const Stop* stop_ = &stops[stops.size() - 1];
	map_all_stops.insert({ stop_->name, stop_ });
	if (!stop_to_bus_map.count(FindStop(stop.name))) {
		stop_to_bus_map[stop_];
	}
}

void TransportCatalogue::SetDistance(std::string_view stop_from, std::string_view stop_to, size_t distance) {
        map_distance_to_stop.insert({{FindStop(stop_from), FindStop(stop_to)}, distance});
}

size_t TransportCatalogue::GetDistance(std::string_view stop_from, std::string_view stop_to) const {
	if (map_distance_to_stop.count({FindStop(stop_from), FindStop(stop_to)})) {
		return map_distance_to_stop.at({FindStop(stop_from), FindStop(stop_to)});
	}
	return map_distance_to_stop.at({FindStop(stop_to), FindStop(stop_from)});
}
    
BusQueryInput TransportCatalogue::GetBusInfo(std::string_view bus_name) const {
    BusQueryInput result;
    if (map_all_buses.count(bus_name)) {
    result.bus_not_found_ = true;
    result.number = FindBus(bus_name)-> name_bus;
    result.stops_count = map_all_buses.at(bus_name)->stop_names.size();
    result.unique_stops_count = CalculateUniqueStops(bus_name);
    result.route_length = CalculateRouteLength(bus_name);
    result.curvature = (result.route_length) / CalculateGeographicLength(bus_name);
    } else {
        result.number = std::string(bus_name);
    }
    return result;
}

StopQueryInput TransportCatalogue::GetStopInfo(std::string_view stop_name) const {
    if (stop_to_bus_map.count(FindStop(stop_name))) {
		return { true, std::string(stop_name), stop_to_bus_map.at(FindStop(stop_name))};
	}
    return {false, std::string(stop_name), {}};
}


double TransportCatalogue::CalculateRouteLength(std::string_view bus_info) const {
    double distance = 0;
		auto from = FindBus(bus_info)->stop_names.begin();
		for (auto to = std::next(from); to != FindBus(bus_info)->stop_names.end(); ++to) {
			distance += GetDistance((*from)->name, (*to)->name);
			from = to;
		}
		return distance;
}

double TransportCatalogue::CalculateGeographicLength(std::string_view bus_info) const {
    double distance = 0;
    geo::Coordinates from = { FindBus(bus_info)->stop_names[0]->coord.lat, FindBus(bus_info)->stop_names[0]->coord.lng };
    for (size_t n = 1; n < FindBus(bus_info)->stop_names.size(); ++n) {
        auto stop = FindBus(bus_info)->stop_names[n];
		geo::Coordinates to {stop->coord.lat, stop->coord.lng};
		distance += ComputeDistance(from, to);
		from = to;
    }
	return distance;
}
    
size_t TransportCatalogue::CalculateUniqueStops(std::string_view bus) const {
	std::unordered_set<const Stop* > unique_stops;
    for (const Stop* stop : FindBus(bus)->stop_names) {
		if (!unique_stops.count(stop)) {
			unique_stops.insert(stop);
		}
	}
    return unique_stops.size();
}

const Stop* TransportCatalogue::FindStop(const std::string_view stop_name) const {
    return map_all_stops.count(stop_name) ? map_all_stops.at(stop_name) : nullptr;
}
    
const Bus* TransportCatalogue::FindBus(const std::string_view bus_name) const {
    return map_all_buses.count(bus_name) ? map_all_buses.at(bus_name) : nullptr;
}

} // namespace transport_catalogue;
