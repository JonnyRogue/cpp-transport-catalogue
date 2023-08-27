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
#include <cstdlib>
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <memory>
#include <filesystem>
#include <fstream>

#include "geo.h"

namespace transport_catalogue {
    
double KmDividedOnTime (double speed);
    
enum class RouteType { 
    CIRCLE, 
    TWO_DIRECTIONAL
};
    
struct Stop {
    std::string name;
    geo::Coordinates coord;
    int id;
};

using PairStop = std::pair<const Stop*, const Stop*>;
using DeqStop = std::deque< Stop*>;
using StringVec = std::vector<std::string>;

struct Bus {
    std::string name_bus;
    DeqStop stop_names;
    RouteType type;
};

struct BusQueryInput {
    std::string number;
    int stops_count = 0;
    int unique_stops_count = 0;
    double route_length = 0.0;
    double curvature = 0.0;
};

struct QueryInputBus {
    std::string name;
    RouteType type;
    std::vector<std::string> stops_list;
};

struct RouteStatistic{

    struct ItemsWait {
        std::string type;
        double time = 0;
        std::string stop_name;
    };

    struct ItemsBus {
        std::string type;
        double time = 0;
        size_t span_count = 0;
        std::string bus;
    };

    using VariantItem = std::variant<ItemsBus, ItemsWait>;

    double total_time = 0;
    std::vector<VariantItem> items;
};
    
}  //transport_catalogue
