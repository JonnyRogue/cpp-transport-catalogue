#include "json_reader.h"

namespace transport_catalogue {
namespace json {

JSONReader::JSONReader(TransportCatalogue& catalogue, request::RequestHandler& request_handler, renderer::MapRenderer& map_renderer) : catalogue_(catalogue), request_handler_(request_handler), map_renderer_(map_renderer) {}

void JSONReader::ReadJSON(std::istream& input) {
    json::Document input_doc = std::move(json::Load(input));
    if (input_doc.GetRoot().IsMap() 
        && input_doc.GetRoot().AsMap().count("base_requests")
		&& input_doc.GetRoot().AsMap().count("stat_requests")
		&& input_doc.GetRoot().AsMap().count("render_settings")) {
            base_requests_ = json::Document{ input_doc.GetRoot().AsMap().at("base_requests") };
			stat_requests_ = json::Document{ input_doc.GetRoot().AsMap().at("stat_requests") };
			map_renderer_.GetRenderSettings() = ReadJsonRenderSettings(input_doc.GetRoot().AsMap().at("render_settings").AsMap());
		} else {
              throw std::logic_error("incorrect input data");
		  } 
}

renderer::RenderSettings JSONReader::ReadJsonRenderSettings(const json::Node& settings_json) const {
    if (settings_json.AsMap().empty()) {
        return renderer::RenderSettings{};
	}
	std::map<std::string, json::Node> s = settings_json.AsMap();
	renderer::RenderSettings settings;
    settings.width = s.at("width").AsDouble();
	settings.height = s.at("height").AsDouble();
	settings.padding = s.at("padding").AsDouble();
	settings.line_width = s.at("line_width").AsDouble();
	settings.stop_radius = s.at("stop_radius").AsDouble();
	settings.bus_label_font_size = s.at("bus_label_font_size").AsInt();
	settings.bus_label_offset.push_back(s.at("bus_label_offset").AsArray()[0].AsDouble());
	settings.bus_label_offset.push_back(s.at("bus_label_offset").AsArray()[1].AsDouble());
	settings.stop_label_font_size = s.at("stop_label_font_size").AsInt();
	settings.stop_label_offset.push_back(s.at("stop_label_offset").AsArray()[0].AsDouble());
	settings.stop_label_offset.push_back(s.at("stop_label_offset").AsArray()[1].AsDouble());
	settings.underlayer_color = ReadUnderlayerColor(s);
	settings.underlayer_width = s.at("underlayer_width").AsDouble();
	settings.color_palette = ReadColorPalette(s);
    return settings;
}

svg::Color JSONReader::ReadUnderlayerColor(const std::map<std::string, json::Node>& s) const {
	svg::Color underlayer_color;
    if (s.at("underlayer_color").IsString()) {
		underlayer_color = s.at("underlayer_color").AsString();
	}
	if (s.at("underlayer_color").IsArray()) {
		if (s.at("underlayer_color").AsArray().size() == 3) {
			underlayer_color = svg::Rgb{
				uint8_t(s.at("underlayer_color").AsArray()[0].AsInt()),
				uint8_t(s.at("underlayer_color").AsArray()[1].AsInt()),
				uint8_t(s.at("underlayer_color").AsArray()[2].AsInt())
			};
		} else {
		      underlayer_color = svg::Rgba{
                  uint8_t(s.at("underlayer_color").AsArray()[0].AsInt()),
				  uint8_t(s.at("underlayer_color").AsArray()[1].AsInt()),
				  uint8_t(s.at("underlayer_color").AsArray()[2].AsInt()),
				  s.at("underlayer_color").AsArray()[3].AsDouble()
			  };
		  }
	}
	return underlayer_color;
}

std::vector<svg::Color> JSONReader::ReadColorPalette(const std::map<std::string, json::Node>& s) const {
	std::vector<svg::Color> color_palette;
	if (s.at("color_palette").AsArray().empty()) {
		color_palette.push_back("none");
	} else {
	      for (const auto& color : s.at("color_palette").AsArray()) {
		      if (color.IsString()) {
                  color_palette.push_back(color.AsString());
			  } else if (color.IsArray() && color.AsArray().size() == 3) {
                    uint8_t r, g, b;
					r = color.AsArray()[0].AsInt();
					g = color.AsArray()[1].AsInt();
					b = color.AsArray()[2].AsInt();
					color_palette.push_back(svg::Rgb{ r, g, b });
				} else {
                      uint8_t r, g, b;
					  double o;
					  r = color.AsArray()[0].AsInt();
					  g = color.AsArray()[1].AsInt();
					  b = color.AsArray()[2].AsInt();
					  o = color.AsArray()[3].AsDouble();
					  color_palette.push_back(svg::Rgba{ r, g, b, o });
				  }
          }
      }
	return color_palette;
}

void JSONReader::BuildDataBase() {
    for (const json::Node& stop_json : base_requests_.GetRoot().AsArray()) {
        if (stop_json.IsMap() && stop_json.AsMap().at("type") == "Stop") {
            AddNameAndCoordinatesOfStop(stop_json);
		}
	}
	for (const json::Node& stop_from : base_requests_.GetRoot().AsArray()) {
		if (stop_from.IsMap() && stop_from.AsMap().at("type") == "Stop") {
			AddDistanceBetweenStops(stop_from);
		}
	}
	for (const json::Node& bus_json : base_requests_.GetRoot().AsArray()) {
		if (bus_json.AsMap().at("type") == "Bus") {
			AddJsonBus(bus_json);
			map_renderer_.AddBus(catalogue_.FindBus(bus_json.AsMap().at("name").AsString()));
		}
	}
}

void JSONReader::AddNameAndCoordinatesOfStop(const json::Node& node) {
    catalogue_.AddStop({node.AsMap().at("name").AsString(), node.AsMap().at("latitude").AsDouble(), node.AsMap().at("longitude").AsDouble()});
}

void JSONReader::AddDistanceBetweenStops(const json::Node& stop_from) {
    for (const auto& stop_to : stop_from.AsMap().at("road_distances").AsMap()) {
		catalogue_.SetDistance(stop_from.AsMap().at("name").AsString(), stop_to.first, stop_to.second.AsInt());
	}
}

void JSONReader::AddJsonBus(const json::Node& node) {
	Bus bus;
	bus.name_bus = node.AsMap().at("name").AsString();
	bus.type = node.AsMap().at("is_roundtrip").AsBool() ? RouteType::CIRCLE: RouteType::TWO_DIRECTIONAL;
    for (const json::Node& stop : node.AsMap().at("stops").AsArray()) {
		bus.stop_names.push_back(catalogue_.FindStop(stop.AsString()));
	}
    if (bus.type == RouteType::TWO_DIRECTIONAL) {
		std::deque<const Stop*> copy = bus.stop_names;
		std::move(std::next(copy.rbegin()), copy.rend(), std::back_inserter(bus.stop_names));
	}
    catalogue_.AddBus(bus);
}

void JSONReader::GenerateAnswer() {
	std::vector<json::Node> answers;
    for (const json::Node& request : stat_requests_.GetRoot().AsArray()) {
		if (request.AsMap().at("type") == "Bus") {
			answers.push_back(json::Node{ GenerateAnswerAboutRoute(request) });
		} else if (request.AsMap().at("type") == "Stop") {
              answers.push_back(json::Node{ GenerateAnswerAboutStop(request) });
		  } else {
                answers.push_back(json::Node{GenerateAnswerAboutMap(request.AsMap().at("id").AsInt())});
			}
	}
	answers_ = std::move(json::Document{ answers });
}

std::map<std::string, json::Node> JSONReader::GenerateAnswerAboutRoute(const json::Node& request) const {
	std::map<std::string, json::Node> bus_info;
	std::string_view bus_name = request.AsMap().at("name").AsString();
    bus_info.insert({ "request_id", json::Node{ request.AsMap().at("id").AsInt() } });
	if (catalogue_.FindBus(bus_name)) {
		bus_info.insert({ "curvature", json::Node{request_handler_.GetBusStat(bus_name)->curvature } });
		bus_info.insert({ "route_length", json::Node{request_handler_.GetBusStat(bus_name)->route_length } });
		bus_info.insert({ "stop_count", json::Node{static_cast<int>(request_handler_.GetBusStat(bus_name)->stops_count) } });
		bus_info.insert({ "unique_stop_count", json::Node{static_cast<int>(request_handler_.GetBusStat(bus_name)->unique_stops_count) } });
	} else {
          bus_info.insert({ "error_message", json::Node{ "not found"}});
	  }
    return bus_info;
}

std::map<std::string, json::Node> JSONReader::GenerateAnswerAboutStop(const json::Node& request) const {
	std::map<std::string, json::Node> stop_info;
	std::string_view stop_name = request.AsMap().at("name").AsString();
    stop_info.insert({ "request_id", request.AsMap().at("id").AsInt() });
	if (catalogue_.FindStop(stop_name)) {
		std::vector<json::Node> buses;
		for (const auto& bus : request_handler_.GetBusesByStop(stop_name)) {
			buses.push_back(bus->name_bus);
		}
		std::sort(buses.begin(), buses.end(), [&](const json::Node& l, const json::Node& r) { return std::lexicographical_compare(l.AsString().begin(), l.AsString().end(), r.AsString().begin(), r.AsString().end()); });
		stop_info.insert({ "buses", buses });
	} else {
          stop_info.insert({ "error_message", "not found" });
	  }
	return stop_info;
}

std::map<std::string, json::Node> JSONReader::GenerateAnswerAboutMap(int id) const {
    std::ostringstream output;
	std::map<std::string, json::Node> answer;
    map_renderer_.RenderMap().Render(output);
    answer.insert({ "request_id", id });
	answer.insert({ "map", output.str()});
    return answer;
}

void JSONReader::PrintAnswer(std::ostream& output) {
	json::Print(answers_, output);
}
    
}//end namespace json
}//end namespace transport_catalogue
