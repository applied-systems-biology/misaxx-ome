//
// Created by rgerst on 13.12.18.
//

#pragma once

#include <misaxx/core/misa_serializeable.h>
#include <misaxx/core/attachments/misa_quantity.h>
#include <misaxx/ome/attachments/misa_ome_unit.h>
#include <misaxx/ome/attachments/misa_ome_voxel_size.h>

namespace misaxx::ome {
    /**
     * Higher-order wrapper around a number that is modeling the number of pixels
     * Can interact with misa_ome_voxel_size to calculate a misa_ome_volume
     */
    struct misa_ome_pixel_count : public misaxx::misa_serializeable, public boost::equality_comparable<misa_ome_pixel_count> {
        long count = 0;

        misa_ome_pixel_count() = default;

        explicit misa_ome_pixel_count(long t_count);

        void from_json(const nlohmann::json &t_json) override;

        void to_json(nlohmann::json &t_json) const override;

        void to_json_schema(const misaxx::misa_json_schema &t_schema) const override;

        misaxx::misa_quantity<double, misa_ome_unit_length<3>> get_volume(const misa_ome_voxel_size &voxel_size) const;

    protected:
        void build_serialization_id_hierarchy(std::vector<misaxx::misa_serialization_id> &result) const override;
    };

    inline void to_json(nlohmann::json& j, const misa_ome_pixel_count& p) {
        p.to_json(j);
    }

    inline void from_json(const nlohmann::json& j, misa_ome_pixel_count& p) {
        p.from_json(j);
    }
}




