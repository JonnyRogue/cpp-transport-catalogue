#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"



using namespace std;
using namespace transport_catalogue;
using namespace transport_catalogue::detail::input; 
using namespace transport_catalogue::detail::output;

int main() {
   TransportCatalogue catalogue;
   InputInformation(catalogue, std::cin);
   PrintFinalResult(catalogue, std::cin);
} 
