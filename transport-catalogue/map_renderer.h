#pragma once

#include "geo.h"
#include "svg.h"
#include "domain.h"

namespace transport_catalogue {
namespace renderer {

inline const double EPSILON = 1e-6;
bool IsZero(double value);
    
struct RenderSettings {
    double width;
    double height;
    double padding;
    double line_width;
    double stop_radius;
    uint32_t bus_label_font_size;
    transport_catalogue::svg::Point bus_label_offset;
    uint32_t stop_label_font_size;
    transport_catalogue::svg::Point stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width;
    std::vector<svg::Color> color_palette;
};

class SphereProjector {
public:
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding) : padding_(padding) {
        if (points_begin == points_end) {
            return;
        }
        const auto [left_it, right_it] = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) { 
            return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;
            const auto [bottom_it, top_it] = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) { 
                return lhs.lat < rhs.lat; });
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
    
    svg::Point operator()(geo::Coordinates coords) const;
    

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};
    
class MapRenderer {
public:
    explicit MapRenderer(const renderer::RenderSettings& render_settings) : settings_(render_settings) {}
    void RenderSvgMap(const transport_catalogue::TransportCatalogue& catalog, std::ostream& out);

private:
    const RenderSettings settings_;
    SphereProjector* projector_ = nullptr;
    const std::map<std::string_view, const transport_catalogue::Bus*>* buses_ = nullptr;
    const std::map<std::string_view, const transport_catalogue::Stop*>* stops_ = nullptr;
    svg::Color GetNextPalleteColor(size_t &color_count) const;
    svg::Color GetPalletColor(size_t route_number) const;
    void RenderLines(svg::Document& svg_doc) const;
    void RenderRouteNames(svg::Document& svg_doc) const;
    void RenderStopCircles(const transport_catalogue::TransportCatalogue& catalog, svg::Document& svg_doc) const;
    void RenderStopNames(const transport_catalogue::TransportCatalogue& catalog, svg::Document& svg_doc) const;
};

} //namespace renderer  
} //namespace transport_catalogue
