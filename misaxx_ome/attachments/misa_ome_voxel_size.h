//
// Created by rgerst on 13.12.18.
//


#pragma once

#include <misaxx/attachments/misa_matrix.h>
#include <ome/xml/meta/OMEXMLMetadata.h>
#include "misa_ome_unit.h"

namespace misaxx_ome {
    /**
     * Higher-order wrapper around a vector of 3 length-unit values, modelling the size of a voxel
     */
    struct misa_ome_voxel_size : public misaxx::misa_serializeable {

        using matrix_type = misaxx::misa_vector<double, misaxx_ome::misa_ome_unit_length <1>, 3>;
        using unit_type = typename matrix_type::unit_type;
        using ome_unit_type = typename unit_type::ome_unit_type;

        /**
         * Matrix that contains
         */
        matrix_type values;

        misa_ome_voxel_size() = default;

        /**
         * Initializes this voxel size from a matrix
         * @param v
         */
        explicit misa_ome_voxel_size(matrix_type v) : values(std::move(v)) {

        }

        /**
         * Initializes the voxel size from metadata of given image series
         * @param t_meta
         * @param t_series
         */
        explicit misa_ome_voxel_size(const ome::xml::meta::OMEXMLMetadata &t_meta, size_t t_series = 0, std::optional<unit_type> t_unit = std::nullopt) {

            auto x = t_meta.getPixelsPhysicalSizeX(t_series);
            auto y = t_meta.getPixelsPhysicalSizeY(t_series);
            auto z = t_meta.getPixelsPhysicalSizeZ(t_series);

            unit_type u;
            if(static_cast<bool>(t_unit)) {
                u = *t_unit;
            }
            else {
                u = x.get_unit();
            }

            // Convert all to the same unit
            if(u != x.getUnit()) {
                x = ome::xml::model::primitives::convert(x, static_cast<ome_unit_type>(u));
            }
            if(x.getUnit() != y.getUnit()) {
                x = ome::xml::model::primitives::convert(y, static_cast<ome_unit_type>(u));
            }
            if(x.getUnit() != z.getUnit()) {
                z = ome::xml::model::primitives::convert(z, static_cast<ome_unit_type>(u));
            }

            // Pass to the matrix
            values = matrix_type({ x.getValue(), y.getValue(), z.getValue() }, x.getUnit());
        }

        /**
         * Returns the element product of the matrix as quantity
         * @return
         */
        auto get_volume() {
            return values.get_element_product();
        }

        auto get_size_x() const {
            return values.get(0, 0);
        }

        auto get_size_y() const {
            return values.get(1, 0);
        }

        auto get_size_z() const {
            return values.get(2, 0);
        }
    };
}