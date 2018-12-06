//
// Created by rgerst on 05.12.18.
//

#pragma once

#include "misa_ome_quantity.h"

namespace misaxx_ome {
    /**
     * Allows attaching the number of pixels
     */
    struct misa_ome_pixels : public misa_ome_length<size_t> {
        misa_ome_pixels() : misa_ome_length<size_t>(0, unit_type::PIXEL) {

        }

        explicit misa_ome_pixels(size_t value) : misa_ome_length<size_t>(value, unit_type::PIXEL) {

        }

        std::vector<misaxx::misa_serialization_id> get_serialization_id_hierarchy() const override {
            return misa_serializeable::create_serialization_id_hierarchy(misaxx::misa_serialization_id("misa_ome", "attachments/pixels"), {
                    misa_ome_length<size_t>::get_serialization_id_hierarchy()
            });
        }
    };

    inline void to_json(nlohmann::json& j, const misa_ome_pixels& p) {
        p.to_json(j);
    }

    inline void from_json(const nlohmann::json& j, misa_ome_pixels& p) {
        p.from_json(j);
    }
}



