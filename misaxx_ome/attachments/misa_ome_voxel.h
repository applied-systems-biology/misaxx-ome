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
    inline misaxx::misa_voxel<misa_ome_length<double>>
    misa_ome_voxel(double x, double y, double z, const misa_ome_unit_length &unit) {
        auto zero = misa_ome_length<double>(0, unit);
        auto x_ = misa_ome_length<double>(x, unit);
        auto y_ = misa_ome_length<double>(y, unit);
        auto z_ = misa_ome_length<double>(z, unit);
        return misaxx::misa_voxel<misa_ome_length<double>>({zero, std::move(x_)}, {zero, std::move(y_)},
                                                           {zero, std::move(z_)});
    }

    /**
     * Helper function that creates a misa_voxel<misa_ome_length<double>> instance from an existing one, but converted to
     * a specific unit
     * @param v
     * @param unit
     * @return
     */
    inline misaxx::misa_voxel<misa_ome_length<double>>
    misa_ome_voxel(misaxx::misa_voxel<misa_ome_length<double>> v, const misa_ome_unit_length &unit) {
        v.range_x.from.convert_to(unit);
        v.range_x.to.convert_to(unit);
        v.range_y.from.convert_to(unit);
        v.range_y.to.convert_to(unit);
        v.range_z.from.convert_to(unit);
        v.range_z.to.convert_to(unit);
        return v;
    }

    /**
     * Loads a voxel from the PhysicalSize attributes in the OME xml
     * @param meta
     * @param series
     * @param unit
     * @return
     */
    inline misaxx::misa_voxel<misa_ome_length<double>>
    misa_ome_voxel(const ome::xml::meta::OMEXMLMetadata &meta, size_t series,
                   const std::optional<misa_ome_unit_length> &unit = std::nullopt) {
        auto x = meta.getPixelsPhysicalSizeX(series);
        auto y = meta.getPixelsPhysicalSizeY(series);
        auto z = meta.getPixelsPhysicalSizeZ(series);
        if (unit) {
            if (x.getUnit() != *unit) {
                x = ome::xml::model::primitives::convert(x, *unit);
            }
            if (y.getUnit() != *unit) {
                y = ome::xml::model::primitives::convert(y, *unit);
            }
            if (z.getUnit() != *unit) {
                z = ome::xml::model::primitives::convert(z, *unit);
            }
        }

        misaxx::misa_range<misa_ome_length<double>> rx(misa_ome_length<double>(0, x.getUnit()),
                                                       misa_ome_length<double>(x.getValue(), x.getUnit()));
        misaxx::misa_range<misa_ome_length<double>> ry(misa_ome_length<double>(0, y.getUnit()),
                                                       misa_ome_length<double>(y.getValue(), y.getUnit()));
        misaxx::misa_range<misa_ome_length<double>> rz(misa_ome_length<double>(0, z.getUnit()),
                                                       misa_ome_length<double>(z.getValue(), z.getUnit()));

        return misaxx::misa_voxel<misa_ome_length<double>>(rx, ry, rz);
    }
}


