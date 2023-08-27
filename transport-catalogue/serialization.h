#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include <transport_catalogue.pb.h>

namespace serializator {

using namespace transport_catalogue;
    
struct SerializatorSettings {
    std::filesystem::path path;
};

class Serializator {
public:
    Serializator(TransportCatalogue& catalogue, TransportRouter& router) : catalogue_(catalogue), router_(router) {}

    void SetSetting(SerializatorSettings settings);

    void SetRendererSettings(const renderer::RenderSettings& settings);

    void SetRouterSettings(const RoutingSettings& settings);

    renderer::RenderSettings GetRenderSettings();

    void Serialize();

    void Deserialize();

private:
    void WriteStops();
    void WriteBuses();
    void WriteDistances();
    void WriteMap();
    void WriteRoutingSettings();
    proto_catalogue::Color SerializeColor(const svg::Color& color);
    
    void ReadStops();
    void ReadBuses();
    void ReadDistances();
    void ReadMap();
    void ReadRoutingSettings();
    svg::Color DeserializeColor(const proto_catalogue::Color &serialized_color);
    
    TransportCatalogue& catalogue_;
    transport_catalogue::TransportRouter& router_;
    SerializatorSettings serialization_settings_;
    renderer::RenderSettings render_settings_;
    transport_catalogue::RoutingSettings routing_settings_;
    proto_catalogue::TransportCatalogue proto_catalogue_;
};

}
