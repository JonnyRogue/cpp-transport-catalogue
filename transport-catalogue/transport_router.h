#pragma once

#include "graph.h"
#include "router.h"
#include "domain.h"
#include "transport_catalogue.h"

namespace transport_catalogue {
    
struct RoutingSettings {
    double bus_wait_time_ = 0; 
    double bus_velocity_ = 0;
};

class TransportRouter {
public:
    using OptRouteInfo = std::optional<graph::Router<double>::RouteInfo>;
    
    RoutingSettings settings_;
    TransportRouter() = default;
    void CreateGraph(TransportCatalogue& db);
    std::optional<RouteStatistic> GetRouteStat(size_t id_stop_from, size_t id_stop_to) const;
    bool IsExist() const;

private:
    struct EdgeAditionInfo {
        std::string bus_name; 
        size_t count_spans = 0;
    };
    
    std::vector<EdgeAditionInfo> edges_buses_;
    std::vector<std::string_view> id_for_stops;
    std::optional<graph::DirectedWeightedGraph<double>> opt_graph_;
    std::unique_ptr<graph::Router<double>> up_router_;
};

} //namespace transport_catalogue 
