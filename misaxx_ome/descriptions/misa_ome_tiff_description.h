//
// Created by rgerst on 19.11.18.
//


#pragma once

#include <misaxx/descriptions/misa_file_description.h>

namespace misaxx_ome {
    struct misa_ome_tiff_description : public misaxx::misa_file_description{
        using misaxx::misa_file_description::misa_file_description;
    };

    void to_json(nlohmann::json& j, const misa_ome_tiff_description& p) {
        p.to_json(j);
    }

    void from_json(const nlohmann::json& j, misa_ome_tiff_description& p) {
        p.from_json(j);
    }
}
