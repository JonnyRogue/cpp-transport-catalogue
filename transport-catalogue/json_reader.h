#pragma once

#include "json.h"
#include "request_handler.h"
#include "map_renderer.h"

namespace transport_catalogue {

struct Data {
    json::Document base_requests;
    json::Document stat_requests;
    renderer::RenderSettings render_settings;
    RoutingSettings routing_settings;
};

class JSONReader {
public:
    JSONReader(TransportCatalogue& catalogue, renderer::MapRenderer& map_renderer);
    Data ReadJSON(std::istream& input) const;
    void BuildDataBase(const Data& data);
    json::Document GenerateAnswer(const TransportRouter& transport_router, const json::Document& stat_requests) const;

private:
    TransportCatalogue& catalogue_;
    renderer::MapRenderer& map_renderer_;
    renderer::RenderSettings render_settings_;
    svg::Color ReadUnderlayerColor(const std::map<std::string, json::Node>& s) const;
    std::vector<svg::Color> ReadColorPalette(const std::map<std::string, json::Node>& s) const;
    renderer::RenderSettings CreateRenderSettings(const json::Node& settings) const;
    RoutingSettings CreateRoutingSettings(const json::Node& input_node) const;
    void AddNameAndCoordinatesOfStop(const json::Node& node);
    void AddDistanceBetweenStops(const json::Node& stop_from);
    void AddJsonBus(const json::Node& node);
    json::Node GenerateAnswerBus(const json::Node& request) const;
    json::Node GenerateAnswerStop(const json::Node& request) const;
    json::Node GenerateAnswerMap(const int id) const;
    json::Node GenerateAnswerRoute(const TransportRouter& router,	const json::Node& request) const;
    json::Node ConvertEdgeInfo(const TransportRouter& router, const EdgeId edge_id) const;
};
    
}//end namespace transport_catalogue
