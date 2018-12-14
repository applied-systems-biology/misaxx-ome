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
    struct misa_ome_voxel_size : public misaxx::misa_serializeable, public boost::equality_comparable<misa_ome_voxel_size> {

        using matrix_type = misaxx::misa_vector<double, misaxx_ome::misa_ome_unit_length <1>, 3>;
        using unit_type = misaxx_ome::misa_ome_unit_length<1>;
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
        explicit misa_ome_voxel_size(matrix_type v);

        explicit misa_ome_voxel_size(double x, double y, double z, unit_type u);

        /**
         * Initializes the voxel size from metadata of given image series
         * @param t_meta
         * @param t_series
         */
        explicit misa_ome_voxel_size(const ome::xml::meta::OMEXMLMetadata &t_meta, size_t t_series = 0, std::optional<unit_type> t_unit = std::nullopt);

        /**
         * Returns the element product of the matrix as quantity
         * @return
         */
        misaxx::misa_quantity<double, misa_ome_unit_length<3>> get_volume() const;

        auto get_size_x() const {
            return values.get(0, 0);
        }

        auto get_size_y() const {
            return values.get(1, 0);
        }

        auto get_size_z() const {
            return values.get(2, 0);
        }

        void from_json(const nlohmann::json &t_json) override;

        void to_json(nlohmann::json &t_json) const override;

        void to_json_schema(const misaxx::misa_json_schema &t_schema) const override;

    protected:

        void build_serialization_id_hierarchy(std::vector<misaxx::misa_serialization_id> &result) const override;

    public:

        bool operator==(const misa_ome_voxel_size &rhs) const;
    };

    inline void to_json(nlohmann::json& j, const misa_ome_voxel_size& p) {
        p.to_json(j);
    }

    inline void from_json(const nlohmann::json& j, misa_ome_voxel_size& p) {
        p.from_json(j);
    }
}