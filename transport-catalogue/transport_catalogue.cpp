#include "transport_catalogue.h" 

namespace transport_catalogue {

void TransportCatalogue::AddBus(const QueryInputBus& query) {
    Bus bus;
    bus.type = query.type;
    bus.name_bus = query.name;
    for (const std::string& st : query.stops_list) {
        Stop* that_stop = FindStop(st);
        bus.stop_names.push_back(that_stop);
    }
    if (bus.type == RouteType::TWO_DIRECTIONAL) {
        for (int i = static_cast<int>(bus.stop_names.size()-2); i >= 0; --i){
            bus.stop_names.push_back(bus.stop_names[i]);
        }
    }
    buses.push_back(std::move(bus));
    map_all_buses[buses.back().name_bus] = &buses.back();

    for (const Stop* stop : map_all_buses[buses.back().name_bus]->stop_names) {
        stop_to_bus_map[stop->name].insert(buses.back().name_bus);
    }
}

void TransportCatalogue::AddStop(std::string_view stop_name, const double lat, const double lng,
const std::vector<std::pair<std::string, double>>& id_){
    if (!map_all_stops.count(stop_name)) {
        Stop the_stop{std::string{stop_name}, lat, lng, id}; 
        stops.push_back(std::move(the_stop));
        map_all_stops[stops.back().name] = &stops.back();
        ++id;
    } else {
          map_all_stops[stop_name] -> coord.lat = lat;
          map_all_stops[stop_name] -> coord.lng = lng;
      }
      Stop* st1 = map_all_stops[stop_name];
      if (!id_.empty()) {
          for (auto& [key, value] : id_){
              if (map_all_stops.count(key)){
                Stop* st2;
                st2 = map_all_stops[key];
                map_distance_to_stop[{st1, st2}] = value;
              } else {
                    Stop st2;
                    st2.id = id;
                    ++ id;
                    st2.name = key;
                    stops.push_back(std::move(st2));
                    map_all_stops[stops.back().name] = &stops.back();
                    map_distance_to_stop[{st1, &stops.back()}] = value;
                }
          }
      }
}
    
void TransportCatalogue::AddBusForSerializator(std::string bus_name, RouteType type, std::vector<std::string> stop_names){
    Bus bus;
    bus.type = type;
    bus.name_bus = bus_name;
    for (auto stop : stop_names) {
        Stop* that_stop = FindStop(stop);
        bus.stop_names.push_back(that_stop);
    }
    buses.push_back(std::move(bus));
    map_all_buses[buses.back().name_bus] = &buses.back();
    for (const Stop* stop : map_all_buses[buses.back().name_bus]->stop_names) {
        stop_to_bus_map[stop->name].insert(buses.back().name_bus);
    }
}

void TransportCatalogue::SetDistance(std::string_view stop_from, std::string_view stop_to, size_t distance) {
        map_distance_to_stop.insert({{FindStop(stop_from), FindStop(stop_to)}, distance});
}

const std::unordered_map<PairStop, double, DistanceHasher> &TransportCatalogue::GetDistance() const {
    return map_distance_to_stop;
}
    
double TransportCatalogue::GetCalculateDistance(const Stop* first_route, const Stop* second_route) {
    if (map_distance_to_stop.count({first_route, second_route}) != 0) {
        return (map_distance_to_stop[{first_route, second_route}]);
    } else {
          return (map_distance_to_stop[{second_route, first_route}]);
      }
}
    
BusQueryInput TransportCatalogue::GetBusInfo(const Bus& bus) const {
    std::set<std::string> buffer_names;
    int stops_count = bus.stop_names.size();
    double length = 0;
    double route_length = 0;
    for (size_t i = 1; i < bus.stop_names.size(); ++i){
        Stop* st1 = bus.stop_names[i-1];
        Stop* st2 = bus.stop_names[i];
        buffer_names.insert(st1 ->name);
        if (map_distance_to_stop.count({st1, st2})){
            route_length += map_distance_to_stop.at({st1, st2});
        } else {
              route_length += map_distance_to_stop.at({st2, st1});
          }
          length += ComputeDistance(bus.stop_names[i-1] -> coord, bus.stop_names[i] -> coord);
    }
    double curvature = route_length / length;
    int unique_stops_count = buffer_names.size();
    BusQueryInput bus_info{bus.name_bus, stops_count, unique_stops_count, route_length, curvature};
    return bus_info;
}
    
std::optional<std::set<std::string>> TransportCatalogue::GetStopInfo(std::string_view query) {
    std::set<std::string> buses_at_stop;
    if (!map_all_stops.count(query)){
        return std::nullopt;
    }
    for (const Bus& that_bus : buses){
        for (Stop* stop : that_bus.stop_names){
            if (stop->name == query){
                buses_at_stop.emplace(that_bus.name_bus);
            }
        }
    }
    return std::optional<std::set<std::string>>{buses_at_stop};
}
       
Stop* TransportCatalogue::FindStop(const std::string_view stop_name) {
    return map_all_stops.count(stop_name) ? map_all_stops.at(stop_name) : nullptr;
}
    
Bus* TransportCatalogue::FindBus(std::string_view bus_name) {
    return map_all_buses.count(bus_name) ? map_all_buses.at(bus_name) : nullptr;
}

const std::map<std::string_view, const Bus*> TransportCatalogue::GetBuses() const {
    std::map<std::string_view, const Bus*>  result(map_all_buses.begin(), map_all_buses.end());
    return result;
}
    
const std::set<std::string_view>& TransportCatalogue::GetBusesForStop(std::string_view stop) const {
    const auto iter = stop_to_bus_map.find(stop);
    if (iter == stop_to_bus_map.end()) {
        return empty_route;
    }
    return iter->second;
}

const std::map<std::string_view, const Stop*> TransportCatalogue::GetStops() const {
    std::map<std::string_view, const Stop*> result(map_all_stops.begin(), map_all_stops.end());
    return result;
}
    
const std::deque<Bus>& TransportCatalogue::GetAllBuses() const {
	return buses;
}

const std::deque<Stop>& TransportCatalogue::GetAllStops() const {
	return stops;
}
    
} // namespace transport_catalogue;
