//
// Created by rgerst on 26.11.18.
//


#pragma once

#include <ome/xml/model/enums/PixelType.h>
#include <ome/files/Types.h>

namespace misaxx_ome::helpers {

    /**
     * Converts an ome pixel type to the number of bits per pixel (per channel)
     * @param pixel_type
     * @return
     */
    inline ome::files::pixel_size_type ome_pixel_type_to_bits_per_pixel(ome::xml::model::enums::PixelType pixel_type) {
        using namespace ome::xml::model::enums;
        switch (pixel_type) {
            case PixelType::UINT8:
                return sizeof(unsigned char) * 8;
            case PixelType::INT8:
                return sizeof(char) * 8;
            case PixelType::UINT16:
                return sizeof(unsigned short) * 8;
            case PixelType::INT16:
                return sizeof(short) * 8;
            case PixelType::UINT32:
                return sizeof(unsigned int) * 8;
            case PixelType::INT32:
                return sizeof(int) * 8;
            case PixelType::FLOAT:
                return sizeof(float) * 8;
            case PixelType::DOUBLE:
                return sizeof(double) * 8;
            default:
                throw std::runtime_error("Unsupported pixel type!");
        }
    }
}