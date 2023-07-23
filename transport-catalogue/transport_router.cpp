#include "transport_router.h"
#include "router.h"

namespace transport_catalogue {

TransportRouter::TransportRouter(const TransportCatalogue& catalogue, RoutingSettings routing_settings) : catalogue_(catalogue) {
	routing_settings_ = std::move(routing_settings);
}

void TransportRouter::BuildGraphAndRouter() {
	BuildVertexes();
    size_t number_all_stops = stop_to_pair_vertex_.size();
	graph_ = std::make_unique<Graph>(Graph(2 * number_all_stops));
	edges_info_.reserve(2 * number_all_stops);
    for (const auto& [stop, pair_vertexes] : stop_to_pair_vertex_) {
		graph_->AddEdge(graph::Edge<double>{
			pair_vertexes.begin_wait, pair_vertexes.end_wait, routing_settings_.bus_wait_time
		});
		edges_info_.push_back(EdgeWaitInfo{ stop, routing_settings_.bus_wait_time });
	}
    for (const Bus* bus : catalogue_.GetAllBuses()) {
		if (bus->type == RouteType::CIRCLE) {
			AddEdgeBusInfo(bus->stop_names.begin(), bus->stop_names.end(), bus);
		} else {
              size_t half = (bus->stop_names.size() + 1) / 2;
			  auto middle_it = std::next(bus->stop_names.begin(), half);
			  auto rmiddle_it = std::next(bus->stop_names.rbegin(), half);
			  AddEdgeBusInfo(bus->stop_names.begin(), middle_it, bus);
			  AddEdgeBusInfo(rmiddle_it - 1, bus->stop_names.rend(), bus);
		  }
    }
	router_ = std::make_unique<Router>(Router(*graph_));
}

void TransportRouter::BuildVertexes() {
	size_t count_vertex = 0;
	for (const Stop* stop : catalogue_.GetAllStops()) {
		stop_to_pair_vertex_.insert({ stop, PairVertexId{ count_vertex++, count_vertex++ } });
	}
}

std::optional<graph::Router<double>::RouteInfo> TransportRouter::BuildRoute(std::string_view stop_from, std::string_view stop_to) const {
	if (!router_) {
		throw std::logic_error("Router no initialization");
	}
    const VertexId from = stop_to_pair_vertex_.at(catalogue_.FindStop(stop_from)).begin_wait;
    const VertexId to = stop_to_pair_vertex_.at(catalogue_.FindStop(stop_to)).begin_wait;
    auto route_info_ptr = router_->BuildRoute(from, to);
	if (!route_info_ptr) {
		return {};
	}
    return router_->BuildRoute(from, to);
}

const EdgeInfo& TransportRouter::GetEdgeInfo(const EdgeId edge_id) const {
	return edges_info_.at(edge_id);
}
} //namespace transport_catalogue 
