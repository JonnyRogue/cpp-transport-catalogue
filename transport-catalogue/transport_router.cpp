#include "transport_router.h"

namespace transport_catalogue {

void TransportRouter::CreateGraph(TransportCatalogue& catalogue) {
    graph::DirectedWeightedGraph<double> graph(catalogue.GetStops().size());
    id_for_stops.resize(catalogue.GetStops().size());
    for(const Bus& bus : catalogue.GetAllBuses()) {
        for (auto it_from = bus.stop_names.begin(); it_from != bus.stop_names.end(); ++it_from) {
            const Stop* stop_from = *it_from;
            double length = 0;
            const Stop* prev_stop = stop_from;
            id_for_stops[stop_from->id] = stop_from->name;
            for (auto it_to = std::next(it_from); it_to != bus.stop_names.end(); ++it_to) {
                const Stop* stop_to = *it_to;
                length += catalogue.GetCalculateDistance(prev_stop, stop_to);
                prev_stop = stop_to;
                double time_on_bus = length / KmDividedOnTime(settings_.bus_velocity_); 
                graph::Edge<double> edge1 { static_cast<graph::VertexId>(stop_from -> id), static_cast<graph::VertexId>(stop_to -> id), time_on_bus+settings_.bus_wait_time_ };
                graph.AddEdge(edge1);
                edges_buses_.push_back({bus.name_bus, static_cast<graph::VertexId>(std::distance(it_from, it_to))});
            }
        }
    }
    opt_graph_ = std::move(graph);
    up_router_ = std::make_unique<graph::Router<double>>(opt_graph_.value());
}

std::optional<RouteStatistic> TransportRouter::GetRouteStat(size_t id_stop_from, size_t id_stop_to) const {
    const OptRouteInfo opt_route_info = up_router_->BuildRoute(id_stop_from, id_stop_to);
    if(! opt_route_info.has_value()) {
        return std::nullopt;
    }
    const graph::Router<double>::RouteInfo& route_info = opt_route_info.value();
    double total_time = route_info.weight;
    std::vector<RouteStatistic::VariantItem> items;
    for(const auto& edge_id : route_info.edges) {
        const auto& edge = opt_graph_.value().GetEdge(edge_id);
        const auto [bus_num, span_count] = edges_buses_[edge_id];
        items.push_back(RouteStatistic::ItemsWait{"Wait", settings_.bus_wait_time_, std::string(id_for_stops[edge.from])});
        items.push_back(RouteStatistic::ItemsBus{"Bus", edge.weight - settings_.bus_wait_time_, span_count, std::string(bus_num)});
    }
    return RouteStatistic{total_time, items};
}

bool TransportRouter::IsExist() const {
    return !opt_graph_.has_value();
}
    
} //namespace transport_catalogue 
