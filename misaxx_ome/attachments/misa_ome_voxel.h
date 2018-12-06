//
// Created by rgerst on 05.12.18.
//

#pragma once


#include <misaxx/attachments/misa_voxel.h>
#include <misaxx_ome/attachments/misa_ome_quantity.h>
#include <ome/xml/meta/OMEXMLMetadata.h>

namespace misaxx_ome {
    /**
     * Helper function that creates a misa_voxel<misa_ome_length<double>> instance
     * @param x
     * @param y
     * @param z
     * @param unit
     * @return
     */
    extern misaxx::misa_voxel<misa_ome_length<double>>
    misa_ome_voxel(double x, double y, double z, const misa_ome_unit_length &unit);

    /**
     * Helper function that creates a misa_voxel<misa_ome_length<double>> instance from an existing one, but converted to
     * a specific unit
     * @param v
     * @param unit
     * @return
     */
    extern misaxx::misa_voxel<misa_ome_length<double>>
    misa_ome_voxel(misaxx::misa_voxel<misa_ome_length<double>> v, const misa_ome_unit_length &unit);

    /**
     * Loads a voxel from the PhysicalSize attributes in the OME xml
     * @param meta
     * @param series
     * @param unit
     * @return
     */
    extern misaxx::misa_voxel<misa_ome_length<double>>
    misa_ome_voxel(const ome::xml::meta::OMEXMLMetadata &meta, size_t series,
                   const std::optional<misa_ome_unit_length> &unit = std::nullopt);
}


