#include "json_reader.h"

using namespace std;
using namespace transport_catalogue;

int main() {

	TransportCatalogue catalogue;
	renderer::MapRenderer map_renderer;
	request::RequestHandler request_handler(catalogue);
	json::JSONReader json_reader(catalogue, request_handler, map_renderer);

	json_reader.ReadJSON(std::cin);
	json_reader.BuildDataBase();
	json_reader.GenerateAnswer();
	json_reader.PrintAnswer(std::cout);
	
}
