//
// Created by rgerst on 17.12.18.
//

#pragma once

#include <misaxx/core/attachments/misa_matrix.h>
#include <misaxx/ome/attachments/misa_ome_unit.h>
#include <misaxx/ome/attachments/misa_ome_voxel_size.h>

namespace misaxx::ome {

    /**
     * Models a cuboid 3D voxel
     * The 'from' point is inclusive, while the 'to' point is exclusive
     */
    struct misa_ome_voxel : public misaxx::misa_serializable {
        using matrix_type = misaxx::misa_matrix<double, misaxx::ome::misa_ome_unit_length <1>, 3, 2>;
        using unit_type = misaxx::ome::misa_ome_unit_length<1>;
        using ome_unit_type = typename unit_type::ome_unit_type;

        /**
         * Matrix that stores the range values.
         * row[0, 1, 2] = x, y, z
         * column[0] = from
         * column[1] = to
         */
        matrix_type ranges;

        /**
         * Initializes an invalid voxel (MAX, MIN) ranges
         */
        misa_ome_voxel();

        /**
         * Initializes an invalid voxel (MAX, MIN) ranges
         * and a unit
         * @param t_unit
         */
        explicit misa_ome_voxel(unit_type t_unit);

        /**
         * Initializes a voxel from an input matrix
         * @param t_matrix
         */
        explicit misa_ome_voxel(matrix_type t_matrix);

        /**
         * Returns the voxel size
         * @return
         */
        explicit operator misa_ome_voxel_size() const;

        /**
         * Returns the voxel size
         * @return
         */
        misa_ome_voxel_size get_size() const;

        /**
         * Returns the size in X direction
         * @return
         */
        misaxx::misa_quantity<double, unit_type> get_size_x() const;

        /**
         * Returns the size in Y direction
         * @return
         */
        misaxx::misa_quantity<double, unit_type> get_size_y() const;

        /**
         * Return the size in Z direction
         * @return
         */
        misaxx::misa_quantity<double, unit_type> get_size_z() const;

        misaxx::misa_quantity<double, unit_type> get_from_x() const;

        misaxx::misa_quantity<double, unit_type> get_to_x() const;

        misaxx::misa_quantity<double, unit_type> get_from_y() const;

        misaxx::misa_quantity<double, unit_type> get_to_y() const;

        misaxx::misa_quantity<double, unit_type> get_from_z() const;

        misaxx::misa_quantity<double, unit_type> get_to_z() const;

        void set_from_x(const misaxx::misa_quantity<double, unit_type> &value);

        void set_to_x(const misaxx::misa_quantity<double, unit_type> &value);

        void set_from_y(const misaxx::misa_quantity<double, unit_type> &value);

        void set_to_y(const misaxx::misa_quantity<double, unit_type> &value);

        void set_from_z(const misaxx::misa_quantity<double, unit_type> &value);

        void set_to_z(const misaxx::misa_quantity<double, unit_type> &value);

        /**
         * Ensures that the point is included in the voxel
         * @param value
         */
        void include_x(const misaxx::misa_quantity<double, unit_type> &value);

        /**
         * Ensures that the point is included in the voxel
         * @param value
         */
        void include_y(const misaxx::misa_quantity<double, unit_type> &value);

        /**
         * Ensures that the point is included in the voxel
         * @param value
         */
        void include_z(const misaxx::misa_quantity<double, unit_type> &value);

        /**
         * Ensures that the point is included in the voxel
         * @param x
         * @param y
         * @param z
         */
        void include(const misaxx::misa_quantity<double, unit_type> &x, const misaxx::misa_quantity<double, unit_type> &y, const misaxx::misa_quantity<double, unit_type> &z);

        void from_json(const nlohmann::json &t_json) override;

        void to_json(nlohmann::json &t_json) const override;

        void to_json_schema(const misaxx::misa_json_schema &t_schema) const override;

        /**
         * Returns true if from < to for all coordinates
         * @return
         */
        bool is_valid() const;

    protected:
        void build_serialization_id_hierarchy(std::vector<misaxx::misa_serialization_id> &result) const override;
    };

    inline void to_json(nlohmann::json& j, const misa_ome_voxel& p) {
        p.to_json(j);
    }

    inline void from_json(const nlohmann::json& j, misa_ome_voxel& p) {
        p.from_json(j);
    }
}




