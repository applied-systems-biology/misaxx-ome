
#include <ome/xml/model/primitives/Quantity.h>
#include <ome/xml/model/enums.h>

int main(const int argc, const char **argv) {
    using namespace ome::xml::model::primitives;
    using namespace ome::xml::model::enums;
    using Length = Quantity<UnitsLength, double>;
}