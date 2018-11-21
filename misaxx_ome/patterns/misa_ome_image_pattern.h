//
// Created by rgerst on 19.11.18.
//


#pragma once

#include <misaxx/misa_data_pattern.h>
#include <misaxx_ome/descriptions/misa_ome_image_description.h>

namespace misaxx_ome {
    class misa_ome_image_pattern : public misaxx::misa_data_pattern<misa_ome_image_description> {
    public:
        void from_json(const nlohmann::json &t_json) override {
            misa_data_pattern_base::from_json(t_json);
        }

        void to_json(nlohmann::json &t_json) const override {
            misa_data_pattern_base::to_json(t_json);
        }

        void to_json_schema(const misaxx::misa_json_schema &t_schema) const override {
            misa_data_pattern_base::to_json_schema(t_schema);
        }

        misaxx::misa_serialization_id get_serialization_id() const override {
            return misaxx::misa_serialization_id("misa_ome", "patterns/ome_image");
        }
    };

    void to_json(nlohmann::json& j, const misa_ome_image_pattern& p) {
        p.to_json(j);
    }

    void from_json(const nlohmann::json& j, misa_ome_image_pattern& p) {
        p.from_json(j);
    }
}
