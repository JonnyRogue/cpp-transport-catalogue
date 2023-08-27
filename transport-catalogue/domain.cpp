#include "domain.h"

namespace transport_catalogue {

double KmDividedOnTime (double speed) {
    return speed * (1000. / 60.);
}

} //transport_catalogue
