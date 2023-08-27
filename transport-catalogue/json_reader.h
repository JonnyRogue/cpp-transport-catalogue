#pragma once

#include "json.h"
#include "request_handler.h"
#include "json_builder.h"
#include "domain.h"
#include "transport_router.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "serialization.h"

namespace transport_catalogue{

inline json::Node GetErrorNode(int id);

svg::Color ParseColor(const json::Node& node);

class JSONReader{
public:
    JSONReader(TransportCatalogue& catalog, TransportRouter& router) : transport_catalogue_(catalog),  router_(router){};

    void MakeBase(std::istream& input);
    void ReadRawJson(std::istream& input, std::vector<json::Document>& document);
    void ParseBase();
    void AddStop(const json::Array& arr);
    void AddBus(const json::Array& arr);
    void AddToCatalog(json::Node node);
    void AddRoutingSettings(const json::Node &root_);
    void ReadRenderSettings(json::Node node);
    void Request(std::istream& input);
    void ParseSerializeSettings();
    void ParseStatRequest();
    void FillMap(int id);
    void FillOutput(json::Node node);
    void FillStop(const std::string& name, int id);
    void FillBus(Bus* bus, int id);
    void FillRout(int id, const json::Dict& request_fields);
    void ReadSerializationSettings(const json::Node &node);
    void OutputInfo(std::ostream& out);
    renderer::RenderSettings GetParsedRenderSettings();
    void SetRenderSettings(const renderer::RenderSettings& settings) ;
    serializator::SerializatorSettings GetSerializatorSettings();
    RoutingSettings GetRoutingSettings();

private:
    TransportCatalogue& transport_catalogue_;
    TransportRouter& router_;
    std::vector<json::Document> base_document_;
    std::vector<json::Document> request_document_;
    std::vector<json::Node> request_to_output_;
    json::Dict settings_;
    renderer::RenderSettings render_settings_;
    serializator::SerializatorSettings serializator_settings_;
    int bus_wait_time_;
    double bus_velocity_;
    
};

} //namespace transport_catalogue
