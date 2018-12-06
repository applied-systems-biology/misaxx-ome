//
// Created by rgerst on 05.12.18.
//

#pragma once

#include <misaxx/attachments/misa_range.h>
#include <misaxx/attachments/misa_location.h>
#include <misaxx_ome/descriptions/misa_ome_plane_description.h>

namespace misaxx_ome {
    /**
     * Attachment allows finding an object via its plane location
     */
    struct misa_ome_location : public misaxx::misa_location {

        /**
         * The planes within the referenced OME TIFF that contains the
         * referenced object
         */
        std::vector<misa_ome_plane_description> planes;

        misa_ome_location() = default;

        void from_json(const nlohmann::json &t_json) override {
            misa_location::from_json(t_json);
            planes = t_json["ome-planes"].get<std::vector<misa_ome_plane_description>>();
        }

        void to_json(nlohmann::json &t_json) const override {
            misa_location::to_json(t_json);
            t_json["ome-planes"] = planes;
        }

        void to_json_schema(const misaxx::misa_json_schema &t_schema) const override {
            misa_location::to_json_schema(t_schema);
            t_schema.resolve("ome-planes").declare_required<std::vector<misa_ome_plane_description>>();
        }

        misaxx::misa_serialization_id get_serialization_id() const override {
            return misaxx::misa_serialization_id("misa_ome", "attachments/ome_location");
        }

        std::vector<misaxx::misa_serialization_id> get_serialization_id_hierarchy() const override {
            return misaxx::misa_serializeable::create_serialization_id_hierarchy(*this, { misaxx::misa_location::get_serialization_id_hierarchy() });
        }

    };

    inline void to_json(nlohmann::json& j, const misa_ome_location& p) {
        p.to_json(j);
    }

    inline void from_json(const nlohmann::json& j, misa_ome_location& p) {
        p.from_json(j);
    }

}


