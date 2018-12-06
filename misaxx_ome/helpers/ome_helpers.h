//
// Created by rgerst on 26.11.18.
//


#pragma once

#include <ome/xml/model/enums/PixelType.h>
#include <ome/files/Types.h>
#include <ome/files/MetadataTools.h>

namespace misaxx_ome::helpers {

    /**
     * Converts an ome pixel type to the number of bits per pixel (per channel)
     * @param pixel_type
     * @return
     */
    extern ome::files::pixel_size_type ome_pixel_type_to_bits_per_pixel(const ome::xml::model::enums::PixelType &pixel_type);

    extern std::shared_ptr<ome::files::CoreMetadata> create_ome_core_metadata(size_t size_X, size_t size_Y, size_t size_Z, size_t size_T, std::vector<size_t> size_C, ome::xml::model::enums::PixelType pixel_type);

    extern std::shared_ptr<ome::xml::meta::OMEXMLMetadata> create_ome_xml_metadata(size_t size_X, size_t size_Y, size_t size_Z, size_t size_T, std::vector<size_t> size_C, ome::xml::model::enums::PixelType pixel_type);
}