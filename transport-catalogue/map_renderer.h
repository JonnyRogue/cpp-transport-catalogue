#pragma once
#include "geo.h"
#include "svg.h"
#include "domain.h"

namespace transport_catalogue {
namespace renderer {

using BusesCoordinates = std::map<const Bus*, std::vector<geo::Coordinates>, CompareBus>;
inline const double EPSILON = 1e-6;
bool IsZero(double value);
    
struct RenderSettings {
    double width = 200;
    double height = 200;
    double padding = 30;
    double line_width = 14;
    double stop_radius = 5;
    int bus_label_font_size = 20;
    std::vector<double> bus_label_offset{7, 15};
    int stop_label_font_size = 20;
    std::vector<double> stop_label_offset{7, -3};
    svg::Color underlayer_color = svg::Rgba{ 255, 255, 255, 0.85 };
    double underlayer_width = 3;
    std::vector<svg::Color> color_palette{ "green", svg::Rgb{ 255,160,0 }, "red" };
};

class SphereProjector {
public:
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding);
    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};
    
class MapRenderer {
public:
    svg::Document RenderMap(const renderer::RenderSettings& render_settings, const std::set< const Bus*, CompareBus >& buses) const;

private:
    std::pair<BusesCoordinates, std::vector<geo::Coordinates>> HighlightBusesAndCoordinatesOfStops(const std::set< const Bus*, CompareBus >& buses) const;
    svg::Document CreateVisual(const renderer::RenderSettings& render_settings, const std::set< const Bus*, CompareBus >& buses, const BusesCoordinates& bus_geo_coords, const SphereProjector projector) const;
    svg::Document CreatePolylinesRoutes(const renderer::RenderSettings& render_settings, const BusesCoordinates& bus_geo_coords, const SphereProjector projector) const;
    void SetPathPropsAttributesPolyline(const renderer::RenderSettings& render_settings, svg::Polyline& polyline, size_t count) const;
    void CreateRouteNames(const renderer::RenderSettings& render_settings, const std::set< const Bus*, CompareBus >& buses, svg::Document& document, const SphereProjector projector) const;
    std::pair<svg::Text, svg::Text> CreateBackgroundAndTitleForRoute(const renderer::RenderSettings& render_settings, svg::Point point, std::string_view bus_name, size_t count) const;
    void CreateStopIcons(const renderer::RenderSettings& render_settings, svg::Document& document, const std::set<const Stop*, CompareStop>& stops, const SphereProjector projector) const;
    void CreateStopNames(const renderer::RenderSettings& render_settings, svg::Document& document, const std::set<const Stop*, CompareStop>& stops, const SphereProjector projector) const;
};

template <typename PointInputIt>
SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding) : padding_(padding) {
    if (points_begin == points_end) {
        return;
    }
    const auto [left_it, right_it] = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
    min_lon_ = left_it->lng;
    const double max_lon = right_it->lng;
    const auto [bottom_it, top_it] = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
    const double min_lat = bottom_it->lat;
    max_lat_ = top_it->lat;
    std::optional<double> width_zoom;
    if (!IsZero(max_lon - min_lon_)) {
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }
    std::optional<double> height_zoom;
    if (!IsZero(max_lat_ - min_lat)) {
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }
    if (width_zoom && height_zoom) {
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    } else if (width_zoom) {
          zoom_coeff_ = *width_zoom;
      } else if (height_zoom) {
            zoom_coeff_ = *height_zoom;
        }
}

} //namespace renderer  
} //namespace transport_catalogue
