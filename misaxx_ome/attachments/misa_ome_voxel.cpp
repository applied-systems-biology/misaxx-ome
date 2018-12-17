//
// Created by rgerst on 17.12.18.
//

#include "misa_ome_voxel.h"
#include <cmath>

using namespace misaxx;
using namespace misaxx_ome;

misa_ome_voxel::misa_ome_voxel() : ranges(matrix_type{ { INFINITY, -INFINITY,
                                                         INFINITY, -INFINITY,
                                                         INFINITY, -INFINITY} }) {

}

misa_ome_voxel::misa_ome_voxel(misa_ome_voxel::unit_type t_unit) :
                                    ranges(matrix_type{ { INFINITY, -INFINITY,
                                                          INFINITY, -INFINITY,
                                                          INFINITY, -INFINITY}, std::move(t_unit) }) {

}

misa_ome_voxel::misa_ome_voxel(misa_ome_voxel::matrix_type t_matrix) : ranges(std::move(t_matrix)) {

}

misa_ome_voxel::operator misa_ome_voxel_size() const {
    return get_size();
}

misa_ome_voxel_size misa_ome_voxel::get_size() const {
    return misa_ome_voxel_size(get_size_x().get_value(), get_size_y().get_value(), get_size_z().get_value(), ranges.get_unit());
}

misa_quantity<double, misa_ome_voxel::unit_type> misa_ome_voxel::get_size_x() const {
    return get_to_x() - get_from_x();
}

misa_quantity<double, misa_ome_voxel::unit_type> misa_ome_voxel::get_size_y() const {
    return get_to_y() - get_from_y();
}

misa_quantity<double, misa_ome_voxel::unit_type> misa_ome_voxel::get_size_z() const {
    return get_to_z() - get_from_z();
}

misa_quantity<double, misa_ome_voxel::unit_type> misa_ome_voxel::get_from_x() const {
    return ranges.get(0, 0);
}

misa_quantity<double, misa_ome_voxel::unit_type> misa_ome_voxel::get_to_x() const {
    return ranges.get(0, 1);
}

misa_quantity<double, misa_ome_voxel::unit_type> misa_ome_voxel::get_from_y() const {
    return ranges.get(1, 0);
}

misa_quantity<double, misa_ome_voxel::unit_type> misa_ome_voxel::get_to_y() const {
    return ranges.get(1, 1);
}

misa_quantity<double, misa_ome_voxel::unit_type> misa_ome_voxel::get_from_z() const {
    return ranges.get(2, 0);
}

misa_quantity<double, misa_ome_voxel::unit_type> misa_ome_voxel::get_to_z() const {
    return ranges.get(2, 1);
}

void misa_ome_voxel::set_from_x(const misa_quantity<double, misa_ome_voxel::unit_type> &value) {
    ranges.set(0, 0, value);
}

void misa_ome_voxel::set_to_x(const misa_quantity<double, misa_ome_voxel::unit_type> &value) {
    ranges.set(0, 1, value);
}

void misa_ome_voxel::set_from_y(const misa_quantity<double, misa_ome_voxel::unit_type> &value) {
    ranges.set(1, 0, value);
}

void misa_ome_voxel::set_to_y(const misa_quantity<double, misa_ome_voxel::unit_type> &value) {
    ranges.set(1, 1, value);
}

void misa_ome_voxel::set_from_z(const misa_quantity<double, misa_ome_voxel::unit_type> &value) {
    ranges.set(2, 0, value);
}

void misa_ome_voxel::set_to_z(const misa_quantity<double, misa_ome_voxel::unit_type> &value) {
    ranges.set(2, 1, value);
}

void misa_ome_voxel::include_x(const misa_quantity<double, misa_ome_voxel::unit_type> &value) {
    if(value < get_from_x()) {
        set_from_x(value);
    }
    if(value >= get_to_x()) {
        set_to_x(value);
    }
}

void misa_ome_voxel::include_y(const misa_quantity<double, misa_ome_voxel::unit_type> &value) {
    if(value < get_from_y()) {
        set_from_y(value);
    }
    if(value >= get_to_y()) {
        set_to_y(value);
    }
}

void misa_ome_voxel::include_z(const misa_quantity<double, misa_ome_voxel::unit_type> &value) {
    if(value < get_from_z()) {
        set_from_z(value);
    }
    if(value >= get_to_z()) {
        set_to_z(value);
    }
}

void misa_ome_voxel::include(const misa_quantity<double, misa_ome_voxel::unit_type> &x,
                             const misa_quantity<double, misa_ome_voxel::unit_type> &y,
                             const misa_quantity<double, misa_ome_voxel::unit_type> &z) {
    include_x(x);
    include_y(y);
    include_z(z);
}

void misa_ome_voxel::from_json(const nlohmann::json &t_json) {
    ranges.from_json(t_json["ranges"]);
}

void misa_ome_voxel::to_json(nlohmann::json &t_json) const {
    misa_serializeable::to_json(t_json);
    ranges.to_json(t_json["ranges"]);
}

void misa_ome_voxel::to_json_schema(const misa_json_schema &t_schema) const {
    ranges.to_json_schema(t_schema.resolve("ranges"));
}

void misa_ome_voxel::build_serialization_id_hierarchy(std::vector<misa_serialization_id> &result) const {
    misa_serializeable::build_serialization_id_hierarchy(result);
    result.emplace_back(misaxx::misa_serialization_id("misa_ome", "attachments/voxel"));
}

bool misa_ome_voxel::is_valid() const {
    return get_from_x() < get_to_x() && get_from_y() < get_to_y() && get_from_z() < get_to_z();
}



