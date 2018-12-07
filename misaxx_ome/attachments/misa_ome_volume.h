//
// Created by rgerst on 07.12.18.
//

#pragma once

#include "misa_ome_quantity.h"

namespace misaxx_ome {

    /**
     * Allows attaching a volume
     */
    struct misa_ome_volume : public misa_ome_length<double> {
        misa_ome_volume() : misa_ome_length<double>(0, unit_type::PIXEL) {

        }

        explicit misa_ome_volume(size_t value, unit_type unit) : misa_ome_length<double>(value, unit) {

        }

        std::vector<misaxx::misa_serialization_id> get_serialization_id_hierarchy() const override {
            return misa_serializeable::create_serialization_id_hierarchy(misaxx::misa_serialization_id("misa_ome", "attachments/volume"), {
                    misa_ome_length<double>::get_serialization_id_hierarchy()
            });
        }
    };

    inline void to_json(nlohmann::json& j, const misa_ome_volume& p) {
        p.to_json(j);
    }

    inline void from_json(const nlohmann::json& j, misa_ome_volume& p) {
        p.from_json(j);
    }
}

