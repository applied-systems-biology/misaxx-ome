//
// Created by rgerst on 29.01.19.
//


#pragma once

#include <misaxx/core/attachments/misa_quantity.h>
#include <misaxx/ome/attachments/misa_ome_unit.h>

namespace misaxx::ome {
    // Convenience type wrappers
    template<typename Value, size_t Order = 1> using misa_ome_length = misaxx::misa_quantity<Value, misa_ome_unit_length<Order>>;
    template<typename Value, size_t Order = 1> using misa_ome_frequency = misaxx::misa_quantity<Value, misa_ome_unit_frequency<Order>>;
    template<typename Value, size_t Order = 1> using misa_ome_power = misaxx::misa_quantity<Value, misa_ome_unit_power<Order>>;
    template<typename Value, size_t Order = 1> using misa_ome_pressure = misaxx::misa_quantity<Value, misa_ome_unit_pressure<Order>>;
    template<typename Value, size_t Order = 1> using misa_ome_temperature = misaxx::misa_quantity<Value, misa_ome_unit_temperature<Order>>;
    template<typename Value, size_t Order = 1> using misa_ome_time = misaxx::misa_quantity<Value, misa_ome_unit_time<Order>>;

    // More convenience wrappers
    template<typename Value> using misa_ome_area = misa_ome_length<Value, 2>;
    template<typename Value> using misa_ome_volume = misa_ome_length<Value, 3>;
    
}