//
// Created by rgerst on 26.11.18.
//


#pragma once

#include <ome/xml/model/enums.h>
#include <misaxx/misa_data_description.h>
#include <misaxx_ome/io/json_ome_pixel_type.h>
#include <ome/files/CoreMetadata.h>
#include <misaxx_ome/helpers/ome_helpers.h>
#include <ome/xml/meta/OMEXMLMetadata.h>
#include <string>

namespace misaxx_ome {
    /**
     * Describes an OME series. This is equivalent to ome::files::CoreMetadata
     * Can be converted from/to native OME data types
     */
    struct misa_ome_series_description : public misaxx::misa_data_description {
        /**
         * Width of a plane in this series
         */
        size_t size_X = 0;

        /**
         * Height of a plane in this series
         */
        size_t size_Y = 0;

        /**
         * Number of planes allocated in Z-axis (depth)
         */
        size_t size_Z = 0;

        /**
         * Number of planes allocated in the time-axis
         */
        size_t size_T = 0;

        /**
         * The size of this vector is the number of planes allocated in the channel-axis
         * The value stored within each array entry determines the number of channels within the plane
         */
        std::vector<ome::files::dimension_size_type> channels;

        /**
         * Pixel type of a channel in each plane
         */
        ome::xml::model::enums::PixelType channel_type = ome::xml::model::enums::PixelType::UINT8;

        using misaxx::misa_data_description::misa_data_description;

        /**
         * Loads this description from the native OME type
         * @param t_ome
         */
        explicit misa_ome_series_description(const ome::files::CoreMetadata &t_ome) : size_X(t_ome.sizeX),
                                                                                      size_Y(t_ome.sizeY),
                                                                                      size_Z(t_ome.sizeZ),
                                                                                      size_T(t_ome.sizeT),
                                                                                      channel_type(t_ome.pixelType),
                                                                                      channels(t_ome.sizeC) {
        }

        /**
         * Loads this description from the native OME type
         * @param t_ome
         */
        explicit misa_ome_series_description(const ome::xml::meta::OMEXMLMetadata &t_ome, ome::files::dimension_size_type series) :
            size_X(t_ome.getPixelsSizeX(series)),
            size_Y(t_ome.getPixelsSizeY(series)),
            size_Z(t_ome.getPixelsSizeZ(series)),
            size_T(t_ome.getPixelsSizeT(series)),
            channel_type(t_ome.getPixelsType(series)) {
            for(size_t channel = 0; channel < t_ome.getChannelCount(series); ++channel) {
                channels.push_back(t_ome.getChannelSamplesPerPixel(series, channel));
            }
        }

        void from_json(const nlohmann::json &t_json) override {
            size_X = t_json["size-x"];
            size_Y = t_json["size-y"];
            size_Z = t_json["size-z"];
            size_T = t_json["size-t"];
            channel_type = t_json["channel-type"];
            channels = t_json["channels"].get<std::vector<ome::files::dimension_size_type>>();
        }

        void to_json(nlohmann::json &t_json) const override {
            t_json["size-x"] = size_X;
            t_json["size-y"] = size_Y;
            t_json["size-z"] = size_Z;
            t_json["size-t"] = size_T;
            t_json["channel-type"] = channel_type;
            t_json["channels"] = channels;
        }

        void to_json_schema(const misaxx::misa_json_schema &t_schema) const override {
            t_schema.resolve("size-x").declare_required<int>();
            t_schema.resolve("size-y").declare_required<int>();
            t_schema.resolve("size-z").declare_required<int>();
            t_schema.resolve("size-t").declare_required<int>();
            t_schema.resolve("channels").declare_required<std::vector<ome::files::dimension_size_type>>();
            std::vector<std::string> channel_types;
            for (const auto &kv : ome::xml::model::enums::PixelType::strings()) {
                channel_types.push_back(kv.first);
            }
            t_schema.resolve("channel-type").declare_required<std::string>(
                    misaxx::misa_json_property<std::string>().make_enum(channel_types));
        }

        std::vector<misaxx::misa_serialization_id> get_serialization_id_hierarchy() const override {
            return misa_serializeable::create_serialization_id_hierarchy(misaxx::misa_serialization_id("misa_ome", "descriptions/ome-series"), {
                    misaxx::misa_data_description::get_serialization_id_hierarchy()
            });
        }

        /**
         * Converts the stored data to the respresentation used by ome::files
         * @return
         */
        inline std::shared_ptr<ome::files::CoreMetadata> as_ome() const {
            std::shared_ptr<ome::files::CoreMetadata> core(std::make_shared<ome::files::CoreMetadata>());
            core->sizeX = size_X;
            core->sizeY = size_Y;
            core->sizeC = channels;
            core->sizeZ = size_Z;
            core->sizeT = size_T;
            core->interleaved = false;
            core->dimensionOrder = ome::xml::model::enums::DimensionOrder::XYZTC;
            core->pixelType = channel_type;
            core->bitsPerPixel = helpers::ome_pixel_type_to_bits_per_pixel(channel_type);
            return core;
        }
    };

    inline void to_json(nlohmann::json &j, const misa_ome_series_description &p) {
        p.to_json(j);
    }

    inline void from_json(const nlohmann::json &j, misa_ome_series_description &p) {
        p.from_json(j);
    }
}
