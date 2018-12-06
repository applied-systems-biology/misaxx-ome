//
// Created by rgerst on 19.11.18.
//


#pragma once

#include <misaxx/descriptions/misa_file_description.h>
#include "misa_ome_series_description.h"

namespace misaxx_ome {

    /**
     * Describes an OME TIFF file
     */
    struct misa_ome_tiff_description : public misaxx::misa_file_description{

        /**
         * Description of the series that are stored within the OME TIFF
         */
        std::vector<misa_ome_series_description> series;

        /**
         * Full metadata storage
         */
        std::shared_ptr<ome::xml::meta::OMEXMLMetadata> metadata;

        using misaxx::misa_file_description::misa_file_description;

        void from_json(const nlohmann::json &t_json) override {
            misa_file_description::from_json(t_json);
            const auto series_json = t_json["series"];
            for(auto it = series_json.begin(); it != series_json.end(); ++it) {
                misa_ome_series_description s;
                s.from_json(it.value());
                series.emplace_back(std::move(s));
            }
            if(t_json.find("ome-xml-metadata") != t_json.end()) {
                metadata = ome::files::createOMEXMLMetadata(t_json["ome-xml-metadata"].get<std::string>());
            }
        }

        void to_json(nlohmann::json &t_json) const override {
            misa_file_description::to_json(t_json);
            for(size_t i = 0; i < series.size(); ++i) {
                series[i].to_json(t_json["series"][std::to_string(i)]);
            }
            t_json["ome-xml-metadata"] = metadata->dumpXML();
        }

        void to_json_schema(const misaxx::misa_json_schema &t_schema) const override {
            misa_file_description::to_json_schema(t_schema);
            for(size_t i = 0; i < series.size(); ++i) {
                series[i].to_json_schema(t_schema.resolve("series", std::to_string(i)));
            }
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
