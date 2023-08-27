#include "serialization.h"

using namespace std::string_literals;

namespace serializator {

using namespace transport_catalogue;

void Serializator::SetSetting(SerializatorSettings settings) {
    serialization_settings_ = settings;
}

void Serializator::SetRendererSettings(const renderer::RenderSettings &settings) {
    render_settings_ = settings;
}

void Serializator::SetRouterSettings(const RoutingSettings& settings){
    routing_settings_ = settings;
}

void Serializator::Serialize() {
    std::ofstream out_file(serialization_settings_.path, std::ios::binary);
    WriteStops();
    WriteBuses();
    WriteDistances();
    WriteMap();
    WriteRoutingSettings();
    proto_catalogue_.SerializeToOstream(&out_file);
}

void Serializator::Deserialize() {
    std::ifstream in_file(serialization_settings_.path, std::ios::binary);
    proto_catalogue_.ParseFromIstream(&in_file);
    ReadStops();
    ReadBuses();
    ReadDistances();
    ReadMap();
    ReadRoutingSettings();
}

void Serializator::WriteStops() {
    const auto stops = catalogue_.GetStops();
    std::vector<proto_catalogue::Stop> serialized_stops_list(stops.size());
    *proto_catalogue_.mutable_stops() = {serialized_stops_list.begin(), serialized_stops_list.end() };
    proto_catalogue_.mutable_stops()->Reserve(stops.size());
    for (const auto& [stop_name, stop] : stops) {
        proto_catalogue::Stop* serialized_stop = proto_catalogue_.mutable_stops(stop->id);
        *serialized_stop->mutable_name() = stop_name;
        serialized_stop->set_lat(stop->coord.lat);
        serialized_stop->set_lng(stop->coord.lng);
        serialized_stop->set_id(stop->id);
    }
}

void Serializator::WriteBuses() {
    for (auto& [bus_name, bus] : catalogue_.GetBuses()) {
        proto_catalogue::Bus* serialized_bus = proto_catalogue_.add_buses();
        *serialized_bus->mutable_name() = bus_name;
        if (bus->type == RouteType::CIRCLE){
            serialized_bus->set_is_roundtrip(true);
        } else {
            serialized_bus->set_is_roundtrip(false);
          }
          for (const auto& stop : bus->stop_names) {
              serialized_bus->add_index_stops(stop->id);
          }
    }
}

void Serializator::WriteDistances() {
    const auto distances = catalogue_.GetDistance();
    for (const auto& [stops, distance] : distances) {
        proto_catalogue::Distance* serialized_distance = proto_catalogue_.add_distances();
        serialized_distance->set_id_stop_first(stops.first->name);
        serialized_distance->set_id_stop_second(stops.second->name);
        serialized_distance->set_distance(distance);
    }
}

void Serializator::WriteMap() {
    proto_catalogue::RenderSettings* serialized_render_settings_ = proto_catalogue_.mutable_render_settings();
    serialized_render_settings_->set_width(render_settings_.width);
    serialized_render_settings_->set_height(render_settings_.height);
    serialized_render_settings_->set_padding(render_settings_.padding);
    serialized_render_settings_->set_line_width(render_settings_.line_width);
    serialized_render_settings_->set_stop_radius(render_settings_.stop_radius);
    serialized_render_settings_->set_bus_label_font_size(render_settings_.bus_label_font_size);
    serialized_render_settings_->set_bus_label_offset_x(render_settings_.bus_label_offset.x);
    serialized_render_settings_->set_bus_label_offset_y(render_settings_.bus_label_offset.y);
    serialized_render_settings_->set_stop_label_font_size(render_settings_.stop_label_font_size);
    serialized_render_settings_->set_stop_label_offset_x(render_settings_.stop_label_offset.x);
    serialized_render_settings_->set_stop_label_offset_y(render_settings_.stop_label_offset.y);
    *serialized_render_settings_->mutable_underlayer_color() = SerializeColor(render_settings_.underlayer_color);
    serialized_render_settings_->set_underlayer_width(render_settings_.underlayer_width);
    for (const svg::Color& color : render_settings_.color_palette) {
        *serialized_render_settings_->add_color_palette() = SerializeColor(color);
    }
}

void Serializator::WriteRoutingSettings() {
    proto_catalogue::RoutingSettings* serialized_routing_settings = proto_catalogue_.mutable_routing_settings();
    serialized_routing_settings->set_bus_wait_time(routing_settings_.bus_wait_time_);
    serialized_routing_settings->set_bus_velocity(routing_settings_.bus_velocity_);
}

void Serializator::ReadRoutingSettings() {
    router_.settings_.bus_wait_time_ = proto_catalogue_.routing_settings().bus_wait_time();
    router_.settings_.bus_velocity_ = proto_catalogue_.routing_settings().bus_velocity();
}

proto_catalogue::Color Serializator::SerializeColor(const svg::Color &color) {
    proto_catalogue::Color serialized_color;
    if (std::holds_alternative<svg::Rgb>(color)) {
        proto_catalogue::RGB* rgb = serialized_color.mutable_rgb();
        rgb->set_red(std::get<svg::Rgb>(color).red);
        rgb->set_green(std::get<svg::Rgb>(color).green);
        rgb->set_blue(std::get<svg::Rgb>(color).blue);
    } else if (std::holds_alternative<svg::Rgba>(color)) {
        proto_catalogue::RGBA* rgba = serialized_color.mutable_rgba();
        rgba->set_red(std::get<svg::Rgba>(color).red);
        rgba->set_green(std::get<svg::Rgba>(color).green);
        rgba->set_blue(std::get<svg::Rgba>(color).blue);
        rgba->set_opacity(std::get<svg::Rgba>(color).opacity);
      } else if (std::holds_alternative<std::string>(color)) {
            serialized_color.set_name_color(std::get<std::string>(color));
        } else {
            serialized_color.set_name_color("monostate"s);
          }
    return serialized_color;
}

void Serializator::ReadStops() {
    for (const auto& stop : proto_catalogue_.stops()) {
        std::vector<std::pair<std::string, double>> id_;
        catalogue_.AddStop(stop.name(), stop.lat(), stop.lng(), id_);
    }
}

void Serializator::ReadBuses() {
    for (const auto& bus : proto_catalogue_.buses()) {
        std::vector<std::string> stop_names;
        for (const auto& id : bus.index_stops()) {
            std::string stop_name = proto_catalogue_.mutable_stops(id)->name();
            stop_names.push_back(move(stop_name));
        }
        RouteType type  = RouteType::CIRCLE;
        if (bus.is_roundtrip()){
            type = RouteType::CIRCLE;
        }
        if (!bus.is_roundtrip()){
            type = RouteType::TWO_DIRECTIONAL;
        }
        catalogue_.AddBusForSerializator(bus.name(), type, stop_names);
    }
}

void Serializator::ReadDistances() {
    for (const auto& distance : proto_catalogue_.distances()) {
        const std::string first_stop_name = distance.id_stop_first();
        const std::string second_stop_name = distance.id_stop_second();
        catalogue_.SetDistance(first_stop_name, second_stop_name, distance.distance());
    }
}

void Serializator::ReadMap() {
    proto_catalogue::RenderSettings* serialized_render_settings = proto_catalogue_.mutable_render_settings();
    render_settings_.width = serialized_render_settings->width();
    render_settings_.height = serialized_render_settings->height();
    render_settings_.padding = serialized_render_settings->padding();
    render_settings_.line_width = serialized_render_settings->line_width();
    render_settings_.stop_radius = serialized_render_settings->stop_radius();
    render_settings_.bus_label_font_size = serialized_render_settings->bus_label_font_size();
    render_settings_.bus_label_offset = {serialized_render_settings->bus_label_offset_x(),
                                         serialized_render_settings->bus_label_offset_y() };
    render_settings_.stop_label_font_size = serialized_render_settings->stop_label_font_size();
    render_settings_.stop_label_offset = {serialized_render_settings->stop_label_offset_x(),
                                          serialized_render_settings->stop_label_offset_y() };
    render_settings_.underlayer_color = DeserializeColor(serialized_render_settings->underlayer_color());
    render_settings_.underlayer_width = serialized_render_settings->underlayer_width();
    for (proto_catalogue::Color serialized_color : serialized_render_settings->color_palette()) {
        render_settings_.color_palette.push_back(DeserializeColor(serialized_color));
    }
}


svg::Color Serializator::DeserializeColor(const proto_catalogue::Color &serialized_color) {
    if (serialized_color.has_rgb()) {
        svg::Rgb rgb;
        rgb.red = serialized_color.rgb().red();
        rgb.green = serialized_color.rgb().green();
        rgb.blue = serialized_color.rgb().blue();
        return rgb;
    } else if (serialized_color.has_rgba()) {
        svg::Rgba rgba;
        rgba.red = serialized_color.rgba().red();
        rgba.green = serialized_color.rgba().green();
        rgba.blue = serialized_color.rgba().blue();
        rgba.opacity = serialized_color.rgba().opacity();
        return rgba;
      } else if (serialized_color.name_color() != "monostate"s) {
            return serialized_color.name_color();
        }
    return std::monostate();
}

renderer::RenderSettings Serializator::GetRenderSettings() {
    return render_settings_;
}

} // namespace serializator
