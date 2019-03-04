//
// Created by rgerst on 19.11.18.
//


#pragma once

#include <misaxx/core/patterns/misa_file_pattern.h>

namespace misaxx::ome {
    struct misa_ome_tiff_pattern : public misaxx::misa_file_pattern {
        misa_ome_tiff_pattern();

        std::string get_documentation_name() const override;

        std::string get_documentation_description() const override;
    };

    inline void to_json(nlohmann::json& j, const misa_ome_tiff_pattern& p) {
        p.to_json(j);
    }

    inline void from_json(const nlohmann::json& j, misa_ome_tiff_pattern& p) {
        p.from_json(j);
    }
}
