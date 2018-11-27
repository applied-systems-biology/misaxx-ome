//
// Created by rgerst on 26.11.18.
//


#pragma once

#include <ome/xml/model/enums.h>
#include <misaxx/misa_data_description.h>
#include <misaxx_ome/io/json_ome_pixel_type.h>
#include <ome/files/CoreMetadata.h>
#include <misaxx_ome/helpers/ome_helpers.h>

namespace misaxx_ome {
    /**
     * Describes an OME series
     */
    struct misa_ome_series_description : public misaxx::misa_data_description {
        int size_x = 0;
        int size_y = 0;
        int num_images = 0;
        ome::xml::model::enums::PixelType channel_type = ome::xml::model::enums::PixelType::UINT8;
        int num_channels = 0;

        using misaxx::misa_data_description::misa_data_description;

        /**
         * Loads this description from the native OME type
         * @param t_ome
         */
        explicit misa_ome_series_description(const ome::files::CoreMetadata &t_ome) : size_x(t_ome.sizeX),
                                                                                      size_y(t_ome.sizeY),
                                                                                      num_images(t_ome.sizeZ),
                                                                                      channel_type(t_ome.pixelType),
                                                                                      num_channels(t_ome.sizeC.at(0)) {
            if (t_ome.sizeC.size() != 1)
                throw std::runtime_error("Unsupported number of channels!");
        }

        void from_json(const nlohmann::json &t_json) override {
            size_x = t_json["size-x"];
            size_y = t_json["size-y"];
            num_images = t_json["num-images"];
            channel_type = t_json["channel-type"];
            num_channels = t_json["num-channels"];
        }

        void to_json(nlohmann::json &t_json) const override {
            t_json["size-x"] = size_x;
            t_json["size-y"] = size_y;
            t_json["num-images"] = num_images;
            t_json["channel-type"] = channel_type;
            t_json["num-channels"] = num_channels;
        }

        void to_json_schema(const misaxx::misa_json_schema &t_schema) const override {
            t_schema.resolve("size-x").declare_required<int>();
            t_schema.resolve("size-y").declare_required<int>();
            t_schema.resolve("num-images").declare_required<int>();
            t_schema.resolve("num-channels").declare_required<int>();
            std::vector<std::string> channel_types;
            for (const auto &kv : ome::xml::model::enums::PixelType::strings()) {
                channel_types.push_back(kv.first);
            }
            t_schema.resolve("channel-type").declare_required<std::string>(
                    misaxx::misa_json_property<std::string>().make_enum(channel_types));
        }

        misaxx::misa_serialization_id get_serialization_id() const override {
            return misaxx::misa_serialization_id("misa_ome", "descriptions/ome-series");
        }

        /**
         * Converts the stored data to the respresentation used by ome::files
         * @return
         */
        inline std::shared_ptr<ome::files::CoreMetadata> as_ome() const {
            std::shared_ptr<ome::files::CoreMetadata> core(std::make_shared<ome::files::CoreMetadata>());
            core->sizeX = size_x;
            core->sizeY = size_y;
            core->sizeC.clear(); // defaults to 1 channel with 1 subchannel; clear this
            core->sizeC.push_back(num_channels);
            core->sizeZ = num_images;
            core->sizeT = 1;
            core->interleaved = false;
            core->dimensionOrder = ome::xml::model::enums::DimensionOrder::XYZTC;
            core->pixelType = channel_type;
            core->bitsPerPixel = helpers::ome_pixel_type_to_bits_per_pixel(channel_type);
            return core;
        }
    };

    void to_json(nlohmann::json &j, const misa_ome_series_description &p) {
        p.to_json(j);
    }

    void from_json(const nlohmann::json &j, misa_ome_series_description &p) {
        p.from_json(j);
    }
}
