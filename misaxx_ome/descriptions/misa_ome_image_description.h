//
// Created by rgerst on 19.11.18.
//


#pragma once

#include <misaxx/misa_data_description.h>

namespace misaxx_ome {
    class misa_ome_image_description : public misaxx::misa_data_description {
    public:
        void from_json(const nlohmann::json &t_json) override {
            misa_data_description::from_json(t_json);
        }

        void to_json(nlohmann::json &t_json) const override {
            misa_data_description::to_json(t_json);
        }

        void to_json_schema(const misaxx::misa_json_schema &t_schema) const override {
            misa_data_description::to_json_schema(t_schema);
        }

        misaxx::misa_serialization_id get_serialization_id() const override {
            return misaxx::misa_serialization_id("misa_ome", "descriptions/ome_image");
        }
    };

    void to_json(nlohmann::json& j, const misa_ome_image_description& p) {
        p.to_json(j);
    }

    void from_json(const nlohmann::json& j, misa_ome_image_description& p) {
        p.from_json(j);
    }
}
