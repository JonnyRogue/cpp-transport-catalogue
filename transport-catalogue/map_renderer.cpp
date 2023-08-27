#include "map_renderer.h"

namespace transport_catalogue {
namespace renderer {

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {(coords.lng - min_lon_) * zoom_coeff_ + padding_, (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
}

void MapRenderer::RenderSvgMap(const TransportCatalogue& catalog, std::ostream& out) {
    const std::map<std::string_view, const Stop*> stops = catalog.GetStops();
    stops_ = &stops;
    std::vector<geo::Coordinates> all_route_stops_coordinates;
    for (const auto& stop : stops) {
        if (catalog.GetBusesForStop(stop.first).empty()) {
            continue;
        }
        all_route_stops_coordinates.push_back(stop.second->coord);
    }
    SphereProjector projector(all_route_stops_coordinates.begin(), 
                              all_route_stops_coordinates.end(),
                              settings_.width, 
                              settings_.height, 
                              settings_.padding);
    projector_ = &projector;
    const std::map<std::string_view, const Bus*> routes = catalog.GetBuses();
    buses_ = &routes;
    svg::Document svg_doc;
    RenderLines(svg_doc);
    RenderRouteNames(svg_doc);
    RenderStopCircles(catalog, svg_doc);
    RenderStopNames(catalog, svg_doc);
    svg_doc.Render(out);
    stops_ = nullptr;
    buses_ = nullptr;
    projector_ = nullptr;
}
    
svg::Color MapRenderer::GetNextPalleteColor(size_t &color_count) const {
    if (color_count >= settings_.color_palette.size()) {
        color_count = 0;
    }
    return settings_.color_palette[color_count++];
}

svg::Color MapRenderer::GetPalletColor(size_t route_number) const {
    if (buses_ == nullptr || route_number >= buses_->size()){
        return {};
    }
    size_t index = route_number % settings_.color_palette.size();
    return settings_.color_palette[index];
}


void MapRenderer::RenderLines(svg::Document &svg_doc) const {
    size_t color_count = 0;
    auto projector = *projector_;
    for (const auto route : *buses_) {
        if (route.second->stop_names.empty()) {
            continue;
        }
        svg::Color palette_color = GetNextPalleteColor(color_count);
        svg::Polyline line;
        line.SetStrokeColor(palette_color)
            .SetFillColor({})
            .SetStrokeWidth(settings_.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        for (auto iter = route.second->stop_names.begin(); iter != route.second->stop_names.end(); ++iter) {
            line.AddPoint(projector((*iter)->coord));
        }
    svg_doc.Add(std::move(line));
    }
}

void MapRenderer::RenderRouteNames(svg::Document& svg_doc) const {
    using namespace std::literals;
    auto projector = *projector_;
    size_t color_count = 0;
    for (auto route : *buses_) {
        if (route.second->stop_names.empty()) {
            continue;
        }
        svg::Text name_start_text;
        name_start_text.SetData(std::string{route.first})
                       .SetPosition(projector(route.second->stop_names.front()->coord))
                       .SetOffset(settings_.bus_label_offset)
                       .SetFontSize(settings_.bus_label_font_size)
                       .SetFontFamily("Verdana"s)
                       .SetFontWeight("bold"s)
                       .SetFillColor(GetNextPalleteColor(color_count));
        svg::Text name_start_plate = name_start_text;
        name_start_plate.SetFillColor(settings_.underlayer_color)
                        .SetStrokeColor(settings_.underlayer_color)
                        .SetStrokeWidth(settings_.underlayer_width)
                        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        svg_doc.Add(name_start_plate);
        svg_doc.Add(name_start_text);\
        size_t middle = route.second->stop_names.size()/2;
        if (route.second->stop_names.front()->name == route.second->stop_names[middle]->name) {
            continue;
        }
        if (route.second->type == RouteType::TWO_DIRECTIONAL) {
            name_start_text.SetPosition(projector(route.second->stop_names[middle]->coord));
            name_start_plate.SetPosition(projector(route.second->stop_names[middle]->coord));
            svg_doc.Add(name_start_plate);
            svg_doc.Add(name_start_text);
        }
    }
}

void MapRenderer::RenderStopCircles(const TransportCatalogue& catalog, svg::Document& svg_doc) const {
    using namespace std::literals;
    auto projector = *projector_;
    for (const auto& stop : *stops_) {
        if (catalog.GetBusesForStop(stop.first).empty()) {
            continue;
        }
        svg::Circle stop_circle;
        stop_circle.SetCenter( projector(stop.second->coord) ).SetRadius(settings_.stop_radius).SetFillColor("white"s);
        svg_doc.Add(stop_circle);
    }
}

void MapRenderer::RenderStopNames(const TransportCatalogue& catalog, svg::Document& svg_doc) const {
    using namespace std::literals;
    auto projector = *projector_;
    for (const auto& stop : *stops_) {
        if (catalog.GetBusesForStop(stop.first).empty()) continue;
        svg::Text stop_name;
        stop_name.SetPosition(projector(stop.second->coord))
                 .SetOffset(settings_.stop_label_offset)
                 .SetFontSize(settings_.stop_label_font_size)
                 .SetFontFamily("Verdana"s)
                 .SetData(std::string{stop.first});
        svg::Text stop_plate = stop_name;
        stop_plate.SetFillColor(settings_.underlayer_color)
                  .SetStrokeColor(settings_.underlayer_color)
                  .SetStrokeWidth(settings_.underlayer_width)
                  .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                  .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        stop_name.SetFillColor("black"s);

        svg_doc.Add(stop_plate);
        svg_doc.Add(stop_name);
    }
}
} //namespace renderer 
} //namespace transport_catalogue
