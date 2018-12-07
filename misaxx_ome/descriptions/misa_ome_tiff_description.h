//
// Created by rgerst on 19.11.18.
//


#pragma once

#include <misaxx/descriptions/misa_file_description.h>
#include <ome/xml/meta/OMEXMLMetadata.h>
#include <ome/files/MetadataTools.h>

namespace misaxx_ome {

    /**
     * Describes an OME TIFF file
     */
    struct misa_ome_tiff_description : public misaxx::misa_file_description{

        /**
         * Full metadata storage
         */
        std::shared_ptr<ome::xml::meta::OMEXMLMetadata> metadata;

        using misaxx::misa_file_description::misa_file_description;

        void from_json(const nlohmann::json &t_json) override {
            misa_file_description::from_json(t_json);
            metadata = ome::files::createOMEXMLMetadata(t_json["ome-xml-metadata"].get<std::string>());
        }

        void to_json(nlohmann::json &t_json) const override {
            misa_file_description::to_json(t_json);
            t_json["ome-xml-metadata"] = metadata->dumpXML();
        }

        void to_json_schema(const misaxx::misa_json_schema &t_schema) const override {
            misa_file_description::to_json_schema(t_schema);
            t_schema.resolve("ome-xml-metadata").declare_optional<std::string>();
        }

        std::vector<misaxx::misa_serialization_id> get_serialization_id_hierarchy() const override {
            return misa_serializeable::create_serialization_id_hierarchy(misaxx::misa_serialization_id("misa_ome", "descriptions/ome-tiff"), {
                    misaxx::misa_data_description::get_serialization_id_hierarchy()
            });
        }
    };

    inline void to_json(nlohmann::json& j, const misa_ome_tiff_description& p) {
        p.to_json(j);
    }

    inline void from_json(const nlohmann::json& j, misa_ome_tiff_description& p) {
        p.from_json(j);
    }
}
