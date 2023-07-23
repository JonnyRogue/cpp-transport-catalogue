#include "json_reader.h"
#include "json_builder.h"
#include "graph.h"

namespace transport_catalogue {

JSONReader::JSONReader(TransportCatalogue& catalogue, renderer::MapRenderer& map_renderer)
    : catalogue_(catalogue), map_renderer_(map_renderer) {}

bool CheckResenceSettings(const json::Node& input_node) {
    return input_node.AsMap().count("base_requests") 
    && input_node.AsMap().count("stat_requests") 
    && input_node.AsMap().count("render_settings") 
    && input_node.AsMap().count("routing_settings");
}
    
Data JSONReader::ReadJSON(std::istream& input) const {
    json::Document input_doc = std::move(json::Load(input));
    const json::Node& input_node = input_doc.GetRoot();
    Data db_documents;
    if (input_node.IsMap() && CheckResenceSettings(input_node)) {
		db_documents.base_requests = std::move(json::Document{
			input_node.AsMap().at("base_requests")
		});
		db_documents.stat_requests = std::move(json::Document{
			input_node.AsMap().at("stat_requests")
		});
		db_documents.render_settings = std::move(CreateRenderSettings(
			input_node.AsMap().at("render_settings").AsMap()
		));
		db_documents.routing_settings = std::move(CreateRoutingSettings(
			input_node.AsMap().at("routing_settings").AsMap()
		));
		} else {
              throw std::logic_error("incorrect input data");
          }
    return db_documents;
}

renderer::RenderSettings JSONReader::CreateRenderSettings(const json::Node& settings_json) const {
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
	settings.bus_label_offset[0] = s.at("bus_label_offset").AsArray()[0].AsDouble();
	settings.bus_label_offset[1] = s.at("bus_label_offset").AsArray()[1].AsDouble();
	settings.stop_label_font_size = s.at("stop_label_font_size").AsInt();
	settings.stop_label_offset[0] = s.at("stop_label_offset").AsArray()[0].AsDouble();
	settings.stop_label_offset[1] = s.at("stop_label_offset").AsArray()[1].AsDouble();
	settings.underlayer_color = ReadUnderlayerColor(s);
	settings.underlayer_width = s.at("underlayer_width").AsDouble();
	settings.color_palette = ReadColorPalette(s);
    return settings;
}

svg::Color JSONReader::ReadUnderlayerColor(const std::map<std::string, json::Node>& s) const {
	svg::Color underlayer_color;
    auto underlayer_col_ = s.at("underlayer_color");
    if (underlayer_col_.IsString()) {
		underlayer_color = underlayer_col_.AsString();
	}
	if (underlayer_col_.IsArray()) { 
		if (underlayer_col_.AsArray().size() == 3) {
			underlayer_color = svg::Rgb{
				uint8_t(underlayer_col_.AsArray()[0].AsInt()),
				uint8_t(underlayer_col_.AsArray()[1].AsInt()),
				uint8_t(underlayer_col_.AsArray()[2].AsInt())
			};
		} else if (underlayer_col_.AsArray().size() == 4) {
              underlayer_color = svg::Rgba{
                  uint8_t(underlayer_col_.AsArray()[0].AsInt()),
				  uint8_t(underlayer_col_.AsArray()[1].AsInt()),
				  uint8_t(underlayer_col_.AsArray()[2].AsInt()),
				  underlayer_col_.AsArray()[3].AsDouble()
			  };
		  }
	}
	return underlayer_color;
}

std::vector<svg::Color> JSONReader::ReadColorPalette(const std::map<std::string, json::Node>& s) const {
    std::vector<svg::Color> color_palette;
    auto color_pal_ = s.at("color_palette").AsArray();
	if (color_pal_.empty()) {
		color_palette.push_back("none");
	} else {
	      for (const auto& color : color_pal_) {
		      if (color.IsString()) {
                  color_palette.push_back(color.AsString());
			  } else if (color.IsArray() && color.AsArray().size() == 3) {
					uint8_t r = color.AsArray()[0].AsInt();
					uint8_t g = color.AsArray()[1].AsInt();
					uint8_t b = color.AsArray()[2].AsInt();
					color_palette.push_back(svg::Rgb{ r, g, b });
				} else if (color.IsArray() && color.AsArray().size() == 4) {
				      uint8_t r = color.AsArray()[0].AsInt();
					  uint8_t g = color.AsArray()[1].AsInt();
					  uint8_t b = color.AsArray()[2].AsInt();
					  double o = color.AsArray()[3].AsDouble();
					  color_palette.push_back(svg::Rgba{ r, g, b, o });
				  }
          }
      }
	return color_palette;
}

void JSONReader::BuildDataBase(const Data& data) {
	render_settings_ = data.render_settings;
    for (const json::Node& json_doc : data.base_requests.GetRoot().AsArray()) {
        if (json_doc.IsMap() && json_doc.AsMap().at("type") == "Stop") {
            AddNameAndCoordinatesOfStop(json_doc);
        }
	}
    for (const json::Node& stop_from : data.base_requests.GetRoot().AsArray()) {
		if (stop_from.IsMap() && stop_from.AsMap().at("type") == "Stop") {
			AddDistanceBetweenStops(stop_from);
		}
	}
    for (const json::Node& bus_json : data.base_requests.GetRoot().AsArray()) {
		if (bus_json.AsMap().at("type") == "Bus") {
			AddJsonBus(bus_json);
		}
	}
}

void JSONReader::AddNameAndCoordinatesOfStop(const json::Node& node) {
    catalogue_.AddStop({
        node.AsMap().at("name").AsString(), 
        node.AsMap().at("latitude").AsDouble(), 
        node.AsMap().at("longitude").AsDouble()
    });
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

json::Document JSONReader::GenerateAnswer(const TransportRouter& transport_router,
	const json::Document& stat_requests) const {
        std::vector<json::Node> answers;
        for (const json::Node& request : stat_requests.GetRoot().AsArray()) {
			if (request.AsMap().at("type") == "Bus") {
                answers.push_back(std::move(GenerateAnswerBus(request)));
            } else if (request.AsMap().at("type") == "Stop") {
                  answers.push_back(std::move(GenerateAnswerStop(request)));
			  } else if (request.AsMap().at("type") == "Route") {
                    answers.push_back(std::move(GenerateAnswerRoute(transport_router, request)));
			    } else {
                      answers.push_back(std::move(GenerateAnswerMap(request.AsMap().at("id").AsInt())));
                  }
        }
    return json::Document{ answers };
}
    
json::Node JSONReader::GenerateAnswerBus(const json::Node& request) const {
	std::string_view bus_name = request.AsMap().at("name").AsString();
	json::builder::Builder answer;
	answer.StartDict().Key("request_id").Value(request.AsMap().at("id").AsInt());
	const BusQueryInput bus_info = std::move(catalogue_.GetBusInfo(bus_name));
	if (catalogue_.FindBus(bus_name)) {
		answer.Key("curvature").Value(bus_info.curvature)
		.Key("route_length").Value(bus_info.route_length)
		.Key("stop_count").Value(static_cast<int>(bus_info.stops_count))
		.Key("unique_stop_count").Value(static_cast<int>(bus_info.unique_stops_count))
		.EndDict();
	} else {
	      answer.Key("error_message").Value("not found").EndDict();
	  }
    return answer.Build();
}

json::Node JSONReader::GenerateAnswerStop(const json::Node& request) const {
	std::string_view stop_name = request.AsMap().at("name").AsString();
	json::builder::Builder answer;
	answer.StartDict().Key("request_id").Value(request.AsMap().at("id").AsInt());
	if (catalogue_.FindStop(stop_name)) {
		answer.Key("buses").StartArray();
        std::vector<json::Node> buses;
		for (const auto& bus : catalogue_.GetStopInfo(stop_name).buses_name) {
			buses.push_back(bus->name_bus);
		}
		std::sort(buses.begin(), buses.end(), [&](const json::Node& l, const json::Node& r) {
			return std::lexicographical_compare(l.AsString().begin(), l.AsString().end(), r.AsString().begin(), r.AsString().end());
		});
		for (const auto& bus : buses) {
			answer.Value(bus.AsString());
		}
		answer.EndArray();
	} else {
	      answer.Key("error_message").Value("not found");
	  }
    answer.EndDict();
    return answer.Build();
}
    
json::Node JSONReader::GenerateAnswerRoute(const TransportRouter& router,
	const json::Node& request) const {
    auto route_info = router.BuildRoute(request.AsMap().at("from").AsString(), request.AsMap().at("to").AsString());
	if (!route_info) {
		return json::builder::Builder{}.StartDict()
        .Key("request_id").Value(request.AsMap().at("id").AsInt())
        .Key("error_message").Value("not found")
        .EndDict().Build();
	}
    json::Array items;
    for (const auto& edge_id : route_info->edges) {
		items.emplace_back(ConvertEdgeInfo(router, edge_id));
	}
    return json::builder::Builder{}.StartDict()
    .Key("request_id").Value(request.AsMap().at("id").AsInt())
	.Key("total_time").Value(route_info->weight)
	.Key("items").Value(items)
	.EndDict().Build();
}

json::Node JSONReader::ConvertEdgeInfo(const TransportRouter& router, const EdgeId edge_id) const {
    if (std::holds_alternative<EdgeBusInfo>(router.GetEdgeInfo(edge_id))) {
		const EdgeBusInfo edge_info = std::get<EdgeBusInfo>(router.GetEdgeInfo(edge_id));
        return json::builder::Builder{}.StartDict()
        .Key("type").Value("Bus")
		.Key("bus").Value(std::string(edge_info.bus->name_bus))
		.Key("span_count").Value(static_cast<int>(edge_info.span_count))
		.Key("time").Value(edge_info.weight)
		.EndDict().Build();
	}
    const EdgeWaitInfo edge_info = std::get<EdgeWaitInfo>(router.GetEdgeInfo(edge_id));
    return json::builder::Builder{}.StartDict()
		   .Key("type").Value("Wait")
		   .Key("stop_name").Value(std::string(edge_info.stop->name))
		   .Key("time").Value(edge_info.weight)
		   .EndDict().Build();
}

json::Node JSONReader::GenerateAnswerMap(const int id) const {
	std::ostringstream output;
    std::set<const Bus*, CompareBus> buses;
	for (const Bus* bus_ptr : catalogue_.GetAllBuses()) {
		buses.insert(bus_ptr);
	}
    map_renderer_.RenderMap(render_settings_, buses).Render(output);
    json::builder::Builder answer;
	answer.StartDict()
	      .Key("request_id").Value(id)
		  .Key("map").Value(output.str())
		  .EndDict();
    return answer.Build();
}

RoutingSettings JSONReader::CreateRoutingSettings(const json::Node& route_settings_node) const {
	double bus_velocity = double(route_settings_node.AsMap().at("bus_velocity").AsInt()) * 1000 / 60;
	double bus_wait = double(route_settings_node.AsMap().at("bus_wait_time").AsInt());
    return RoutingSettings{ bus_velocity, bus_wait };
}
    
}//end namespace transport_catalogue
