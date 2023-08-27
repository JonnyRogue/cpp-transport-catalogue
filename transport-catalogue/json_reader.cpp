#include "json_reader.h"
#include "json_builder.h"
#include "graph.h"

using namespace std::literals;

namespace transport_catalogue {

void JSONReader::ReadSerializationSettings(const json::Node &node) {
    serializator_settings_.path = node.AsMap().at("file"s).AsString();
}

void JSONReader::OutputInfo(std::ostream& out){
    json::Array output_array;
    for (json::Node element : request_to_output_){
        output_array.push_back(element);
    }
    json::Print (json::Document { json::Builder{}.Value(output_array).Build() }, out);
}
    
void JSONReader::MakeBase(std::istream& input){
    JSONReader::ReadRawJson(input, base_document_);
    JSONReader::ParseBase();
}

void JSONReader::ReadRawJson(std::istream& input, std::vector<json::Document>& document) {
    json::Document doc = json::Load(input);
    if (doc.GetRoot().IsMap()) {
        document.emplace_back(std::move(doc));
    }
}

void JSONReader::ParseBase() {
    for (auto& doc : base_document_){
        json::Node raw_map = doc.GetRoot();
        if (!raw_map.IsMap()){
            throw json::ParsingError("Incorrect input data type");
        }
        for(const auto& [key, value] : raw_map.AsMap()){
            if (key == "base_requests"){
                AddToCatalog(raw_map.AsMap().at(key));
            } else if (key == "render_settings"){
                  ReadRenderSettings(raw_map.AsMap().at(key));
              } else if (key == "routing_settings") {
                    AddRoutingSettings(raw_map.AsMap().at(key));
                } else if (key == "serialization_settings") {
                      ReadSerializationSettings(raw_map.AsMap().at(key));
                  }
            }
        }
}
    
void JSONReader::Request(std::istream& input) {
    JSONReader::ReadRawJson(input, request_document_);
    JSONReader::ParseSerializeSettings();
}

void JSONReader::ParseStatRequest() {
    for (auto& doc : request_document_){
        json::Node raw_map = doc.GetRoot();
        if (!raw_map.IsMap()){
            throw json::ParsingError("Incorrect input data type");
        }
        for(const auto& [key, value] : raw_map.AsMap()){
            if (key == "stat_requests"){
                FillOutput(raw_map.AsMap().at(key));
            }
        }
    }
}
    
void JSONReader::ParseSerializeSettings() {
    for (auto& doc : request_document_){
        json::Node raw_map = doc.GetRoot();
        if (!raw_map.IsMap()){
            throw json::ParsingError("Incorrect input data type");
        }
        for(const auto& [key, value] : raw_map.AsMap()){
            if (key == "serialization_settings"){
                ReadSerializationSettings(raw_map.AsMap().at(key));
            }
        }
    }
}
    
void JSONReader::AddRoutingSettings(const json::Node &root) {
    for (auto [key, value] : root.AsMap()){
        if (key == "bus_wait_time"){
            router_.settings_.bus_wait_time_ = value.AsDouble();
        } else if (key == "bus_velocity"){
              router_.settings_.bus_velocity_ = value.AsDouble();
          }
    }
}
    
void JSONReader::AddStop(const json::Array& arr) {
    for (auto& element : arr){
        const json::Dict& dict = element.AsMap();
        const auto type_i = dict.find("type"s);
        if (type_i == dict.end()) {
            continue;
        }
        if (type_i->second == "Stop"s) {
            Stop stop;
            std::vector<std::pair<std::string, double>> id_;
            if (const auto name_i = dict.find("name"s); name_i != dict.end() && name_i->second.IsString()) {
                stop.name = name_i->second.AsString();
            } else {
                  continue;
              }
            if (const auto lat_i = dict.find("latitude"s); lat_i != dict.end() && lat_i->second.IsDouble()) {
                stop.coord.lat = lat_i->second.AsDouble();
            } else {
                  continue;
              }
            if (const auto lng_i = dict.find("longitude"s); lng_i != dict.end() && lng_i->second.IsDouble()) {
                stop.coord.lng = lng_i->second.AsDouble();
            } else {
                  continue;
              }
            const auto dist_i = dict.find("road_distances"s);
            if (dist_i != dict.end() && !(dist_i->second.IsMap())) {
                continue;
            }
            for (const auto& [other_name, other_dist] : dist_i->second.AsMap()) {
                if (!other_dist.IsInt()) {
                    continue; 
                }
                id_.push_back({other_name, static_cast<size_t>(other_dist.AsInt())});
            }
            transport_catalogue_.AddStop(stop.name, stop.coord.lat, 
            stop.coord.lng, id_);
        }
    }
}
    
void JSONReader::AddBus(const json::Array& arr) {
    for (auto& element : arr) {
        const json::Dict& dict = element.AsMap();
        const auto type_i = dict.find("type"s);
        if (type_i == dict.end()) {
            continue;
        }
        if (type_i->second == "Bus"s) {
            QueryInputBus bus;
            if (const auto name_i = dict.find("name"s); name_i != dict.end() && name_i->second.IsString()) {
                bus.name = name_i->second.AsString();
            } else {
                  continue;
              }
            if (const auto route_i = dict.find("is_roundtrip"s); route_i != dict.end() && route_i->second.IsBool()) {
                bus.type = route_i->second.AsBool() ? RouteType::CIRCLE : RouteType::TWO_DIRECTIONAL;
            } else{
                  continue;
              }
            const auto stops_i = dict.find("stops"s);
            if (stops_i != dict.end() && !(stops_i->second.IsArray())) {
                continue;
            }
            for (const auto& stop_name : stops_i->second.AsArray()) {
                if (!stop_name.IsString()) {
                    continue;
                }
                bus.stops_list.emplace_back(stop_name.AsString());
            }
            transport_catalogue_.AddBus(bus);
        } else {
            continue;
        }
    }
}
    
void JSONReader::AddToCatalog(json::Node node) {
    if (!node.IsArray()) {
        throw json::ParsingError("Incorrect input data type");
    }
    const json::Array& arr = node.AsArray();
    AddStop(arr);
    AddBus(arr);
}
    
void JSONReader::FillMap(int id) {
    renderer::MapRenderer svg_map(render_settings_);
    std::ostringstream stream;
    svg_map.RenderSvgMap(transport_catalogue_, stream);
    json::Dict result;
    result.emplace("request_id"s, id);
    result.emplace("map"s, std::move(stream.str()));
    request_to_output_.push_back(json::Node(result));
}
    
void JSONReader::FillRout(int id, const json::Dict& request_fields) {
    json::Array out;
    std::string stop_from;
    std::string stop_to;
    if (const auto from_i = request_fields.find("from"s); from_i != request_fields.end() && from_i->second.IsString()) {
        stop_from = from_i->second.AsString();
    }
    if (const auto to_i = request_fields.find("to"s); to_i != request_fields.end()){
        stop_to = to_i -> second.AsString();
    }
    int id_from = transport_catalogue_.FindStop(stop_from) -> id;
    int id_to = transport_catalogue_.FindStop(stop_to) -> id;
    auto get_find_route = router_.GetRouteStat(id_from, id_to);
    if (get_find_route == std::nullopt) {
        json::Node dict_node_stop{json::Dict{{"request_id"s,    id},
                                             {"error_message"s, "not found"s}}};
        request_to_output_.push_back({dict_node_stop});
        return;
    }
    json::Array items;
    for (const auto &get_f_r : get_find_route -> items) {
        json::Dict dict;
        if(std::holds_alternative<RouteStatistic::ItemsWait>(get_f_r)) {
            auto it = std::get<RouteStatistic::ItemsWait>(get_f_r);
            dict.insert({"stop_name"s, it.stop_name});
            dict.insert({"time"s, it.time});
            dict.insert({"type"s, it.type});
        } else if (std::holds_alternative<RouteStatistic::ItemsBus>(get_f_r)) {
            auto it = std::get<RouteStatistic::ItemsBus>(get_f_r);
            dict.insert({"bus"s, it.bus});
            dict.insert({"span_count"s, static_cast<int>(it.span_count)});
            dict.insert({"time"s, it.time});
            dict.insert({"type"s, it.type});
        }
        items.push_back(dict);
    }
    json::Dict rout_stat_dict;
    rout_stat_dict.insert({"items", items});
    rout_stat_dict.insert({"request_id", id});
    rout_stat_dict.insert({"total_time", get_find_route -> total_time});
    request_to_output_.push_back(rout_stat_dict);
}

void JSONReader::FillStop(const std::string& name, int id) {
    json::Dict result;
    json::Array buses;
    const std::optional<std::set<std::string>>& bus_routes = transport_catalogue_.GetStopInfo(name);
    for (auto bus_route : *bus_routes) {
        buses.emplace_back(std::string{bus_route});
    }
    result.emplace("request_id"s, id);
    result.emplace("buses"s, buses);
    request_to_output_.push_back(json::Node(result));
}

void JSONReader::FillBus(Bus* bus, int id) {
    BusQueryInput info = transport_catalogue_.GetBusInfo(*bus);
    json::Dict result;
    result.emplace("request_id"s, id);
    result.emplace("curvature"s, info.curvature);
    result.emplace("route_length"s, static_cast<int>(info.route_length));
    result.emplace("stop_count"s, static_cast<int>(info.stops_count));
    result.emplace("unique_stop_count"s, static_cast<int>(info.unique_stops_count));
    request_to_output_.push_back(json::Node(result));
}

void JSONReader::FillOutput(json::Node request) {
    if (!request.IsArray()){
        throw json::ParsingError("Incorrect input data type");
    }
    const json::Array& arr = request.AsArray();
    for (auto& element : arr){
        if (!element.IsMap()) {
            throw json::ParsingError("One of request nodes is not a dictionary.");
        }
        const json::Dict& request_fields = element.AsMap();
        int id = -1;
        if (const auto id_i = request_fields.find("id"s); id_i != request_fields.end() && id_i->second.IsInt()) {
            id = id_i->second.AsInt();
        } else{
              throw json::ParsingError("Invalid field in request' node");
          }
        const auto type_i = request_fields.find("type"s);
        if ( type_i == request_fields.end() || !(type_i->second.IsString()) ){
            throw json::ParsingError("Invalid field in request' node");
        }
        std::string type = type_i->second.AsString();
        if ( type == "Map"s) {
            FillMap(id);
            continue;
        } else if (type == "Route"s){
              if (router_.IsExist()) { 
                  router_.CreateGraph(transport_catalogue_); 
              }
              FillRout(id, request_fields); 
              continue;
          }
        std::string name;
        if (const auto name_i = request_fields.find("name"s); name_i != request_fields.end() && name_i->second.IsString()) {
            name = name_i->second.AsString();
        } else {
              throw json::ParsingError("Invalid field in request' node");
          }
        if ( type == "Bus"s) {
            Bus* bus = transport_catalogue_.FindBus(name);
            if (!bus){
                request_to_output_.push_back(GetErrorNode(id));
                continue;
            }
            FillBus(bus, id);
        } else if (type == "Stop"s) {
              if (!(transport_catalogue_.FindStop(name)) ) {
                  request_to_output_.push_back(GetErrorNode(id));
                  continue;
              }
              FillStop(name, id);
          } else{
                throw json::ParsingError("Invalid stat request.");
            } 
    }
}
                   
void JSONReader::ReadRenderSettings(json::Node node) {
    if (!node.IsMap()) {
        throw json::ParsingError("Error reading JSON data with render settings.");
    }
    settings_ = node.AsMap();
    render_settings_ = GetParsedRenderSettings();
}

renderer::RenderSettings JSONReader::GetParsedRenderSettings(){
    renderer::RenderSettings settings;
    settings.width = settings_.at("width"s).AsDouble();
    settings.height = settings_.at("height"s).AsDouble();
    settings.padding = settings_.at("padding"s).AsDouble();
    settings.line_width = settings_.at("line_width"s).AsDouble();
    settings.stop_radius = settings_.at("stop_radius"s).AsDouble();
    settings.bus_label_font_size = settings_.at("bus_label_font_size"s).AsInt();
    settings.stop_label_font_size = settings_.at("stop_label_font_size"s).AsInt();
    settings.underlayer_width = settings_.at("underlayer_width"s).AsDouble();
    if (const auto field_iter = settings_.find("bus_label_offset"s); field_iter != settings_.end() && field_iter->second.IsArray()) {
        json::Array arr = field_iter->second.AsArray();
        if (arr.size() != 2) throw json::ParsingError("Invaild bus label font offset data.");
        settings.bus_label_offset.x = arr[0].AsDouble();
        settings.bus_label_offset.y = arr[1].AsDouble();
    } else {
          throw json::ParsingError("Invaild bus label font offset data.");
      }
    if (const auto field_iter = settings_.find("stop_label_offset"s); field_iter != settings_.end() && field_iter->second.IsArray()) {
        json::Array arr = field_iter->second.AsArray();
        if (arr.size() != 2) throw json::ParsingError("Invaild stop label font offset data.");
        settings.stop_label_offset.x = arr[0].AsDouble();
        settings.stop_label_offset.y = arr[1].AsDouble();
    } 
    else{
        throw json::ParsingError("Invaild stop label font offset data.");
    }
    if (const auto field_iter = settings_.find("underlayer_color"s); field_iter != settings_.end() ) {
        svg::Color color = ParseColor(field_iter->second);
        if (std::holds_alternative<std::monostate>(color)) {
            throw json::ParsingError("Invaild underlayer color data.");
        }
        settings.underlayer_color = color;
    } 
    else{
        throw json::ParsingError("Invaild underlayer color data.");
    }
    if (const auto field_iter = settings_.find("color_palette"s); field_iter != settings_.end() && field_iter->second.IsArray()) {
        json::Array arr = field_iter->second.AsArray();
        for (const auto& color_node : arr) {
            svg::Color color = ParseColor(color_node);
            if (std::holds_alternative<std::monostate>(color)) {
                throw json::ParsingError("Invaild color palette data.");
            }
            settings.color_palette.emplace_back(color);
        }
    }
    else{
        throw json::ParsingError("Invaild color palette data.");
    }
    return settings;
}

void JSONReader::SetRenderSettings(const renderer::RenderSettings &settings)
{
    render_settings_ = settings;
}

serializator::SerializatorSettings JSONReader::GetSerializatorSettings()
{
    return serializator_settings_;
}

RoutingSettings JSONReader::GetRoutingSettings()
{
    return router_.settings_;
}

inline json::Node GetErrorNode(int id) {
    json::Dict result;
    result.emplace("request_id"s, id);
    result.emplace("error_message"s, "not found"s);
    return {result};
}

svg::Color ParseColor(const json::Node& node){
    if (node.IsString()) {
        return {node.AsString()};
    }
    if (node.IsArray()) {
        json::Array arr = node.AsArray();
        if (arr.size() == 3) {
            uint8_t red = arr[0].AsInt();
            uint8_t green = arr[1].AsInt();
            uint8_t blue = arr[2].AsInt();
            return { svg::Rgb(red, green, blue) };
        }
        if (arr.size() == 4) {
            uint8_t red = arr[0].AsInt();
            uint8_t green = arr[1].AsInt();
            uint8_t blue = arr[2].AsInt();
            double opacity = arr[3].AsDouble();
            return { svg::Rgba(red, green, blue, opacity) };
        }
    }
    return {};
}
    
}//end namespace transport_catalogue
