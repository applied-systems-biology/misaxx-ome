//
// Created by rgerst on 03.12.18.
//


#pragma once

#include <ome/xml/model/enums/PixelType.h>
#include <misaxx/utils/type_traits.h>

namespace misaxx_ome {

    /**
     * Converts a coi++ image type to an OME pixel type
     * @tparam Image
     * @return
     */
    template<class Image> inline ome::xml::model::enums::PixelType coixx_to_ome_pixel_type() {
        using channel_type = typename Image::color_type::channel_type;

        if constexpr (std::is_same<channel_type, unsigned char>::value) {
            return ome::xml::model::enums::PixelType::UINT8;
        }
        else if constexpr (std::is_same<channel_type, char>::value) {
            return ome::xml::model::enums::PixelType::INT8;
        }
        else if constexpr (std::is_same<channel_type, unsigned short>::value) {
                return ome::xml::model::enums::PixelType::UINT16;
        }
        else if constexpr (std::is_same<channel_type, short>::value) {
            return ome::xml::model::enums::PixelType::INT16;
        }
        else if constexpr (std::is_same<channel_type, unsigned int>::value) {
            return ome::xml::model::enums::PixelType::UINT32;
        }
        else if constexpr (std::is_same<channel_type, int>::value) {
            return ome::xml::model::enums::PixelType::INT32;
        }
        else if constexpr (std::is_same<channel_type, float>::value) {
            return ome::xml::model::enums::PixelType::FLOAT;
        }
        else if constexpr (std::is_same<channel_type, double>::value) {
            return ome::xml::model::enums::PixelType::DOUBLE;
        }
        else {
            static_assert(misaxx::utils::always_false<Image>::value, "Unsupported image type!");
        }
    }

}