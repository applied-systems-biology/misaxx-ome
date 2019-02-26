//
// Created by rgerst on 05.12.18.
//

#pragma once

#include <misaxx/core/attachments/misa_location.h>
#include <misaxx/ome/descriptions/misa_ome_plane_description.h>

namespace misaxx::ome {
    /**
     * Attachment allows finding an object via its plane location
     */
    struct misa_ome_planes_location : public misaxx::misa_location {

        /**
         * The planes within the referenced OME TIFF that contains the
         * referenced object
         */
        std::vector<misa_ome_plane_description> planes;

        using misaxx::misa_location::misa_location;

        explicit misa_ome_planes_location(misaxx::misa_cached_data_base &t_cache, std::vector<misa_ome_plane_description> t_planes);

        void from_json(const nlohmann::json &t_json) override;

        void to_json(nlohmann::json &t_json) const override;

        void to_json_schema(misaxx::misa_json_schema_property &t_schema) const override;

    protected:
        void build_serialization_id_hierarchy(std::vector<misaxx::misa_serialization_id> &result) const override;
    };

    inline void to_json(nlohmann::json& j, const misa_ome_planes_location& p) {
        p.to_json(j);
    }

    inline void from_json(const nlohmann::json& j, misa_ome_planes_location& p) {
        p.from_json(j);
    }

}



