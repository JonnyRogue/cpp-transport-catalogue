#include "stat_reader.h"
#include "input_reader.h"

namespace transport_catalogue { 
namespace detail {
namespace output {
    
void PrintBusInfo(std::ostream& output, const BusQueryInput& bus_info) {
    if(bus_info.bus_not_found_) {
        output << "Bus " << bus_info.number << ": "
		         << bus_info.stops_count << " stops on route, "
		         << bus_info.unique_stops_count << " unique stops, "
                         << std::setprecision(6)
			 << bus_info.route_length << " route length, "
			 << bus_info.curvature << " curvature" << std::endl;
    } else {
         output << "Bus " << bus_info.number << ": not found" << std::endl;
      }
}

void PrintStopInfo(std::ostream& output, const StopQueryInput& stop_info) {
    if(!stop_info.not_found_) {
        output << "Stop " << stop_info.name << ": not found" << std::endl;
    } else if (stop_info.not_found_ && stop_info.buses_name.empty()) {
          output << "Stop " << stop_info.name << ": no buses" << std::endl;
      } else {
            output << "Stop " << stop_info.name << ": buses ";
            std::string str;
			for (auto bus : stop_info.buses_name) {
				str += std::string(bus) + " ";
			}
			str = str.substr(0, str.find_last_not_of(' ') + 1);
            std::cout << str << std::endl;
        }
}

std::string_view ParseRequest(std::string_view output_info) {
    std::string_view name = (output_info[0] == 'B') ? output_info.substr(3) : output_info.substr(4);
    size_t begin = name.find_first_not_of(' ');
	size_t end = name.find_last_not_of(' ');
	name = name.substr(begin, end + 1 - begin);
    return name;
}

void PrintFinalResult(const TransportCatalogue& catalogue, std::istream& input) {
        /*size_t request_count = 0;
        input >> request_count;
        input.ignore();

        for(size_t i = 0; i < request_count; ++i)
        {
            std::string request;
            std::getline(input, request);
            
            std::string_view request_type = ParseRequest(request);

            if(request_type == "Bus")
            {
                BusQueryInput bus_info = catalogue.GetBusInfo(request);
                PrintBus(std::cout, bus_info);
            }
            if(request_type == "Stop")
            {
                StopQueryInput stop_info = catalogue.GetStopInfo(request);
                PrintStop(std::cout, stop_info);*/
    std::vector<std::string> request = std::move(input::ReadInputData(input));
    for (const std::string& query : request) {
        if (query[0] == 'B') {
            std::string_view name = ParseRequest(query);
			BusQueryInput bus_info = catalogue.GetBusInfo(name);
			PrintBusInfo(std::cout, bus_info);
        } else {
              std::string_view name = ParseRequest(query);
			  StopQueryInput stop_info = catalogue.GetStopInfo(name);
			  PrintStopInfo(std::cout, stop_info);
          }
    }
}
    
}//namespace output
}//namespace detail
}//namespace transport_catalogue 
