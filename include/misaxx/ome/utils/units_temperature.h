//
// Created by rgerst on 23.01.19.
//


#pragma once

#include <misaxx/ome/attachments/misa_ome_unit.h>

namespace misaxx::ome::units {
    template<int Order = 1> misa_ome_unit_temperature<Order> celsius() {
        return misa_ome_unit_temperature<Order>(::ome::xml::model::enums::UnitsTemperature::CELSIUS);
    }

    template<int Order = 1> misa_ome_unit_temperature<Order> fahrenheit() {
        return misa_ome_unit_temperature<Order>(::ome::xml::model::enums::UnitsTemperature::FAHRENHEIT);
    }

    template<int Order = 1> misa_ome_unit_temperature<Order> kelvin() {
        return misa_ome_unit_temperature<Order>(::ome::xml::model::enums::UnitsTemperature::KELVIN);
    }

    template<int Order = 1> misa_ome_unit_temperature<Order> rankine() {
        return misa_ome_unit_temperature<Order>(::ome::xml::model::enums::UnitsTemperature::RANKINE);
    }
}