#include "domain.h"

namespace transport_catalogue {
    
bool CompareBus::operator()(const Bus* l, const Bus* r) const {
	return std::lexicographical_compare(l->name_bus.begin(), l->name_bus.end(), r->name_bus.begin(), r->name_bus.end());
}

bool CompareStop::operator()(const Stop* l, const Stop* r) const {
	return std::lexicographical_compare(l->name.begin(), l->name.end(), r->name.begin(), r->name.end());
}
    
}  //transport_catalogue
