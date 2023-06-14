#include "map_renderer.h"

namespace transport_catalogue {
namespace renderer {

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {(coords.lng - min_lon_) * zoom_coeff_ + padding_, (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
}

RenderSettings& MapRenderer::GetRenderSettings() {
    return render_settings_;
}

void MapRenderer::AddBus(const Bus* bus) {
    buses_.insert(bus);
}

void MapRenderer::CreateStopIcons(svg::Document& document, const std::set<const Stop*, CompareStop>& stops, const SphereProjector projector) const {
    svg::Circle circle;
    circle.SetRadius(render_settings_.stop_radius).SetFillColor("white");
    for (const Stop* stop : stops) {
        circle.SetCenter(projector({ stop->coord.lat, stop->coord.lng }));
        document.Add(circle);
    }
}

void MapRenderer::CreateStopNames(svg::Document& document, const std::set<const Stop*, CompareStop>& stops, const SphereProjector projector) const {
    svg::Text background;
    svg::Text title;
    for (const Stop* stop : stops) {
        background.SetPosition(projector({ stop->coord.lat, stop->coord.lng }))
        .SetOffset({ render_settings_.stop_label_offset[0], render_settings_.stop_label_offset[1] })
        .SetFontSize(render_settings_.stop_label_font_size)
        .SetFontFamily("Verdana")
        .SetData(stop->name)
        .SetFillColor(render_settings_.underlayer_color)
        .SetStrokeColor(render_settings_.underlayer_color)
        .SetStrokeWidth(render_settings_.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        
        title.SetPosition(projector({ stop->coord.lat, stop->coord.lng }))
        .SetOffset({ render_settings_.stop_label_offset[0], render_settings_.stop_label_offset[1] })
        .SetFontSize(render_settings_.stop_label_font_size)
        .SetFontFamily("Verdana")
        .SetData(stop->name)
        .SetFillColor("black");
        document.Add(background);
        document.Add(title);
    }
}
    
svg::Document MapRenderer::CreateVisual(const BusesCoordinates& bus_geo_coords, const SphereProjector projector) const {
    svg::Document document = CreatePolylinesRoutes(bus_geo_coords, projector);
    CreateRouteNames(document, projector);
    std::set<const Stop*, CompareStop> stops;
    for (const Bus* bus : buses_) {
        for (const Stop* stop : bus->stop_names) {
            stops.insert(stop);
        }
    } 
    CreateStopIcons(document, stops, projector);
    CreateStopNames(document, stops, projector);
    return document;
}
    
svg::Document MapRenderer::RenderMap() const {
    auto [map_buses_to_geo_coords_of_stops, all_geo_coords] = HighlightBusesAndCoordinatesOfStops();
    const SphereProjector proj = {all_geo_coords.begin(), 
                                  all_geo_coords.end(),
                                  render_settings_.width, 
                                  render_settings_.height,
                                  render_settings_.padding };
    svg::Document document = std::move(CreateVisual(map_buses_to_geo_coords_of_stops, proj));
    return document;
}

std::pair<BusesCoordinates, std::vector<geo::Coordinates>> MapRenderer::HighlightBusesAndCoordinatesOfStops() const {
    BusesCoordinates bus_geo_coords;
    std::vector<geo::Coordinates> all_geo_coords;
    for (const Bus* bus : buses_) {
        std::vector<geo::Coordinates> geo_coords;
        for (const auto& stop : bus->stop_names) {
            geo_coords.push_back({ stop->coord.lat, stop->coord.lng });
            all_geo_coords.push_back({ stop->coord.lat, stop->coord.lng });
        }
        bus_geo_coords.insert({ bus, std::move(geo_coords) });
    }
    return std::pair{ bus_geo_coords, all_geo_coords };
}

svg::Document MapRenderer::CreatePolylinesRoutes(const BusesCoordinates&  bus_geo_coords, const SphereProjector projector) const {
    svg::Document document;
    size_t count = 0;
    for (const auto& [bus, geo_coords] : bus_geo_coords) {
        if (bus->stop_names.empty()) {
            continue;
        }
        svg::Polyline polyline;
        for (const auto& geo_coord : geo_coords) {
            polyline.AddPoint(projector(geo_coord));
        }
        SetPathPropsAttributesPolyline(polyline, count);
        document.Add(polyline);
        ++count;
    }
    return document;
}

void MapRenderer::SetPathPropsAttributesPolyline(svg::Polyline& polyline, size_t count) const {
    polyline.SetFillColor(svg::NoneColor)
    .SetStrokeColor(render_settings_.color_palette
    .at(count % render_settings_.color_palette.size()))
    .SetStrokeWidth(render_settings_.line_width)
    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
}

std::pair<svg::Text, svg::Text> MapRenderer::CreateBackgroundAndTitleForRoute(svg::Point point, std::string_view bus_name, size_t count) const {
    svg::Text background;
    svg::Text title;
    background.SetPosition(point)
    .SetOffset({ render_settings_.bus_label_offset[0], render_settings_.bus_label_offset[1] })
    .SetFontSize(render_settings_.bus_label_font_size)
    .SetFontFamily("Verdana")
    .SetFontWeight("bold")
    .SetData(std::string(bus_name))
    .SetFillColor(render_settings_.underlayer_color)
    .SetStrokeColor(render_settings_.underlayer_color)
    .SetStrokeWidth(render_settings_.underlayer_width)
    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    
    title.SetPosition(point)
    .SetOffset({ render_settings_.bus_label_offset[0], render_settings_.bus_label_offset[1] })
    .SetFontSize(render_settings_.bus_label_font_size)
    .SetFontFamily("Verdana")
    .SetFontWeight("bold")
    .SetData(std::string(bus_name))
    .SetFillColor(render_settings_.color_palette
    .at(count % render_settings_.color_palette.size()));
    
    return {background, title};
}
    
void MapRenderer::CreateRouteNames(svg::Document& document, const SphereProjector proj) const {
    size_t count = 0;
    for (const Bus* bus : buses_) {
        if (bus->stop_names.empty()) {
            continue;
        }
        geo::Coordinates route_begin = { bus->stop_names[0]->coord.lat, bus->stop_names[0]->coord.lng };
        geo::Coordinates route_end = { bus->stop_names[(bus->stop_names.size())/2]->coord.lat, bus->stop_names[(bus->stop_names.size())/2]->coord.lng };
        auto [background_begin, title_begin] = std::move(CreateBackgroundAndTitleForRoute(proj(route_begin), bus->name_bus, count));
        document.Add(background_begin);
        document.Add(title_begin);
        if (bus->type == RouteType::TWO_DIRECTIONAL && route_begin != route_end) {
            auto [background_end, title_end] = std::move(CreateBackgroundAndTitleForRoute(proj(route_end), bus->name_bus, count));
            document.Add(background_end);
            document.Add(title_end);
        }
        ++count;
    }
}

} //namespace renderer 
} //namespace transport_catalogue
