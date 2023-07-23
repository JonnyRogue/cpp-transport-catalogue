#include "json.h"
#include "json_reader.h"
#include "request_handler.h"
#include "transport_router.h"

int main() {
    using namespace std;
    using namespace transport_catalogue;
    using namespace transport_catalogue::json;
    
    TransportCatalogue catalogue;
    renderer::MapRenderer map_renderer;
    JSONReader json_reader(catalogue, map_renderer);
    Data data = std::move(json_reader.ReadJSON(std::cin));
    TransportRouter transport_router(catalogue, data.routing_settings);
    RequestHandler request_handler(catalogue, map_renderer, transport_router);
    json_reader.BuildDataBase(data);
    transport_router.BuildGraphAndRouter();
    Document answers = std::move(json_reader.GenerateAnswer(transport_router, data.stat_requests));
    Print(answers, std::cout);
	
    return 0;
}
