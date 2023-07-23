#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"

namespace transport_catalogue {
    using EdgeId = size_t;
	using VertexId = size_t;
    
struct EdgeWaitInfo {
    const Stop* stop = nullptr;
    double weight = 0;
};

struct EdgeBusInfo {
	const Bus* bus = nullptr;
	double weight = 0;
	size_t span_count = 0;
};

using EdgeInfo = std::variant<EdgeWaitInfo, EdgeBusInfo>;

struct RoutingSettings {
	double bus_velocity = 1000; 
	double bus_wait_time = 6;
};

struct PairVertexId {
	VertexId begin_wait;
	VertexId end_wait;
};

class TransportRouter {
	using VertexId = size_t;
	using Graph = graph::DirectedWeightedGraph<double>;
	using Router = graph::Router<double>;
	
public:
	TransportRouter(const TransportCatalogue& catalogue, RoutingSettings routing_settings);
    void BuildGraphAndRouter();
    std::optional<graph::Router<double>::RouteInfo> BuildRoute(
	std::string_view stop_from, std::string_view stop_to) const;
    const EdgeInfo& GetEdgeInfo(const EdgeId edge_id) const;

private:
	const TransportCatalogue& catalogue_;
	std::unique_ptr<Graph> graph_;
	std::unique_ptr<Router> router_;
	RoutingSettings routing_settings_;
	std::unordered_map<const Stop*, PairVertexId> stop_to_pair_vertex_;
	std::vector<EdgeInfo> edges_info_;
    void BuildVertexes();
    template<typename Iter>
	void AddEdgeBusInfo(Iter first, Iter last, const Bus* bus);
};

template<typename Iter>
void TransportRouter::AddEdgeBusInfo(Iter first, Iter last, const Bus* bus) {
    for (auto from = first; from != last; ++from) {
		size_t distance = 0;
		size_t span_count = 0;
		VertexId from_vertex = stop_to_pair_vertex_.at(*from).end_wait;
		for (auto to = std::next(from); to != last; ++to) {
			auto before_to = std::prev(to);
			distance += catalogue_.GetDistance((*before_to)->name, (*to)->name);
			++span_count;
            VertexId to_vertex = stop_to_pair_vertex_.at(*to).begin_wait;
			double weight = double(distance) / routing_settings_.bus_velocity;
            graph_->AddEdge(graph::Edge<double>{ from_vertex, to_vertex, weight });
			edges_info_.push_back(EdgeBusInfo{ bus, weight, span_count });
		}
	}
}

} //namespace transport_catalogue 
