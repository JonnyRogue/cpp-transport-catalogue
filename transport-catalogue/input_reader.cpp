#include "input_reader.h"
#include "stat_reader.h"

namespace transport_catalogue {
namespace detail {
namespace input {
    
std::string ReadLine(std::istream& input) {
    std::string str;
    std::getline(input, str);
    return str;
}

std::vector<std::string> ReadInputData(std::istream& input) {
    std::string str;
    size_t number_requaests = std::stoi(ReadLine(input));
    std::vector<std::string> queries;
    queries.reserve(number_requaests);
    for (size_t count = 1; count <= number_requaests; ++count) {
        std::string line = std::move(ReadLine(input));
        line = line.substr(line.find_first_not_of(' '));
        queries.push_back(std::move(line));
    }
    return queries;
}

std::pair<std::string_view, std::string_view> SplitFunc(std::string_view line, char sign) {
    size_t pos = line.find(sign);
    std::string_view left = line.substr(0, pos);
    left = left.substr(0, left.find_last_not_of(' ') + 1);
    if (pos < line.size() && pos + 1 < line.size()) {
        size_t begin = line.find_first_not_of(' ', pos + 1);
	size_t end = line.find_last_not_of(' ') + 1;
	std::string_view right = line.substr(begin, end - begin);
	return {left, right};
    } else {
          return {left, std::string_view()};
      }
}
    
std::pair<std::vector<Stop>, std::unordered_map<std::string, std::string>> SplitQuery(
    const StringVec& queries) {
    std::pair<std::vector<Stop>, std::unordered_map<std::string, std::string>> info;
    info.first.reserve(queries.size());
    Stop stop;
    for (const std::string& query : queries) {
        if (query[0] == 'S') {
			std::string query_stop = query.substr(4); 
			auto [name, data] = std::move(SplitFunc(query_stop, ':'));
			stop.name = std::string(name.substr(name.find_first_not_of(' ')));
			auto [lat, data1] = std::move(SplitFunc(data, ','));
            stop.coord.lat = std::stod(std::string(lat));
			auto [lng, data2] = std::move(SplitFunc(data1, ','));
			stop.coord.lng = std::stod(std::string(lng));
            info.first.push_back(stop);
			if (!data2.empty()) {
                info.second[stop.name] = std::move(std::string(data2));
			}
		}
	}
	return info;
}

Bus ParseRoute(const TransportCatalogue& catalogue, std::string_view route, char sign) {
    auto [name, stops] = std::move(SplitFunc(route, ':'));
    name = name.substr(name.find_first_not_of(' '));
	DeqStop stops_ = SplitStopsInRoute(catalogue, stops, sign);
    if (sign == '-') {
        DeqStop copy = stops_;
		std::move(std::next(copy.rbegin()), copy.rend(), std::back_inserter(stops_));
	}
    RouteType type = (sign == '>') ? RouteType::CIRCLE : RouteType::TWO_DIRECTIONAL;
    return {type, std::string(name), stops_};
}

std::unordered_map<std::string, size_t> ParseLine(std::string_view line) {
    if (line.empty()) {
		return {};
	}
	std::unordered_map<std::string, size_t> info;
	std::pair<std::string_view, std::string_view> str_v;
	str_v.second = line;
    while (str_v.second.find(',') != std::string_view::npos) {
		str_v = SplitFunc(str_v.second, ',');
		auto [distance, stop] = SplitFunc(str_v.first, 'm');
		info[std::string(stop.substr(3))] = std::stoi(std::string(distance));
	}
	auto [distance, stop] = SplitFunc(str_v.second, 'm');
	info[std::string(stop.substr(3))] = std::stoi(std::string(distance));
    return info;
}

DeqStop SplitStopsInRoute(const TransportCatalogue& catalogue, std::string_view& stops, char sign) {
	DeqStop stops_;
	const Stop* stop_ = nullptr;
    std::string_view line_stops = stops;
    size_t number = std::count(stops.begin(), stops.end(), sign);
    for (size_t count = 1; count <= number; ++count) {
        auto [name, line] = SplitFunc(line_stops, sign);
        stop_ = catalogue.FindStop(name);
		stops_.push_back(stop_);
        line_stops = line;
	}
	stop_ = catalogue.FindStop(line_stops);
	stops_.push_back(stop_);
    return stops_;
}

Bus InfoRoute(const TransportCatalogue& catalogue, const std::string& query_bus) {
    std::string route = query_bus.substr(3); 
    if (route.find('-') != std::string::npos) {
		return ParseRoute(catalogue, route, '-');
	}
	return ParseRoute(catalogue, route, '>');
}
   
std::vector<Bus> SplitBuses(const TransportCatalogue& catalogue, const StringVec& queries) {
    std::vector<Bus> buses;
	buses.reserve(queries.size() / 2);
    for (const std::string& query : queries) {
        if (query[0] == 'B') {
            buses.push_back(std::move(InfoRoute(catalogue, query)));
        }

	}
    return buses;
}

void InputInformation(TransportCatalogue& catalogue, std::istream& input) {
    StringVec querie = std::move(ReadInputData(input));
	auto [info_first, info_second] = SplitQuery(querie);
	for (const Stop& stop : info_first) {
        catalogue.AddStop(stop);
	}
    for (auto [stop_from, line] : info_second) {
		std::unordered_map<std::string, size_t> stops = ParseLine(line);
		for (const auto& [stop_to, distance] : stops) {
			catalogue.SetDistance(stop_from, stop_to, distance);
		}
	}
	std::vector<Bus> buses = std::move(SplitBuses(catalogue, querie));
	for (const Bus& bus : buses) {
		catalogue.AddBus(bus);
	}
}

}//namespace input
}//namespace detail
}//namespace transport_catalogue 
