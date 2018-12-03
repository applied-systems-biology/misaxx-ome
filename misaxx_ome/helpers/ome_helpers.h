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
    inline ome::files::pixel_size_type ome_pixel_type_to_bits_per_pixel(const ome::xml::model::enums::PixelType &pixel_type) {
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

    inline std::shared_ptr<ome::files::CoreMetadata> create_ome_core_metadata(size_t size_X, size_t size_Y, size_t size_Z, size_t size_T, std::vector<size_t> size_C, ome::xml::model::enums::PixelType pixel_type) {
        std::shared_ptr<ome::files::CoreMetadata> core(std::make_shared<ome::files::CoreMetadata>());
        core->sizeX = size_X;
        core->sizeY = size_Y;
        core->sizeC = std::move(size_C);
        core->sizeZ = size_Z;
        core->sizeT = size_T;
        core->interleaved = false;
        core->dimensionOrder = ome::xml::model::enums::DimensionOrder::XYZTC;
        core->pixelType = pixel_type;
        core->bitsPerPixel = ome_pixel_type_to_bits_per_pixel(pixel_type);
        return core;
    }

    inline std::shared_ptr<ome::xml::meta::OMEXMLMetadata> create_ome_xml_metadata(size_t size_X, size_t size_Y, size_t size_Z, size_t size_T, std::vector<size_t> size_C, ome::xml::model::enums::PixelType pixel_type)
    {
        /* create-metadata-start */
        // OME-XML metadata store.
        auto meta = std::make_shared<ome::xml::meta::OMEXMLMetadata>();

        // Create simple CoreMetadata and use this to set up the OME-XML
        // metadata.  This is purely for convenience in this example; a
        // real writer would typically set up the OME-XML metadata from an
        // existing MetadataRetrieve instance or by hand.
        std::vector<std::shared_ptr<ome::files::CoreMetadata>> seriesList = { create_ome_core_metadata(size_X, size_Y, size_Z, size_T, std::move(size_C), pixel_type) };

        ome::files::fillMetadata(*meta, seriesList);
        /* create-metadata-end */

        return meta;
    }
}