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
#include <variant>

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
    std::unordered_set<const Bus*> buses_name;
};
    
struct CompareBus {
public:
	bool operator()(const Bus* l, const Bus* r) const;
};
	
struct CompareStop {
public:
    bool operator()(const Stop* l, const Stop* r) const;
};
    
}  //transport_catalogue
