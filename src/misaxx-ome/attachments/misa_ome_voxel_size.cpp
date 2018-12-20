//
// Created by rgerst on 13.12.18.
//

#include <misaxx-ome/attachments/misa_ome_voxel_size.h>

using namespace misaxx_ome;

misa_ome_voxel_size::misa_ome_voxel_size(misa_ome_voxel_size::matrix_type v) : values(std::move(v)) {

}

misa_ome_voxel_size::misa_ome_voxel_size(double x, double y, double z, misa_ome_voxel_size::unit_type u) :
values(matrix_type{{ x, y, z }, std::move(u)}) {

}

misa_ome_voxel_size::misa_ome_voxel_size(const ome::xml::meta::OMEXMLMetadata &t_meta, size_t t_series,
                                         std::optional<misa_ome_voxel_size::unit_type> t_unit) {

    using length = ome::xml::model::primitives::Quantity<ome::xml::model::enums::UnitsLength, double>;

    auto x_ = t_meta.getPixelsPhysicalSizeX(t_series);
    auto y_ = t_meta.getPixelsPhysicalSizeY(t_series);
    auto z_ = t_meta.getPixelsPhysicalSizeZ(t_series);

    length x(x_.getValue(), x_.getUnit());
    length y(y_.getValue(), y_.getUnit());
    length z(z_.getValue(), z_.getUnit());

    unit_type u;
    if(static_cast<bool>(t_unit)) {
        u = *t_unit;
    }
    else {
        u = x.getUnit();
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

void misa_ome_voxel_size::from_json(const nlohmann::json &t_json) {
    values.from_json(t_json["values"]);
}

void misa_ome_voxel_size::to_json(nlohmann::json &t_json) const {
    misa_serializeable::to_json(t_json);
    values.to_json(t_json["values"]);
}

void misa_ome_voxel_size::to_json_schema(const misaxx::misa_json_schema &t_schema) const {
    values.to_json_schema(t_schema.resolve("values"));
}


bool misa_ome_voxel_size::operator==(const misa_ome_voxel_size &rhs) const {
    return values == rhs.values;
}

void misa_ome_voxel_size::build_serialization_id_hierarchy(std::vector<misaxx::misa_serialization_id> &result) const {
    misa_serializeable::build_serialization_id_hierarchy(result);
    result.emplace_back(misaxx::misa_serialization_id("misa_ome", "attachments/voxel-size"));
}

misaxx::misa_quantity<double, misa_ome_unit_length<3>> misa_ome_voxel_size::get_volume() const {
    return values.get_element_product();
}

misaxx::misa_quantity<double, misa_ome_unit_length<2>> misa_ome_voxel_size::get_xy_area() const {
    return get_size_x() * get_size_y();
}

misa_ome_unit_length<1> misa_ome_voxel_size::get_unit() const {
    return values.get_unit();
}

misaxx::misa_quantity<double, misa_ome_unit_length<1>> misa_ome_voxel_size::get_size_x() const {
    return values.get(0, 0);
}

misaxx::misa_quantity<double, misa_ome_unit_length<1>> misa_ome_voxel_size::get_size_y() const {
    return values.get(1, 0);
}

misaxx::misa_quantity<double, misa_ome_unit_length<1>> misa_ome_voxel_size::get_size_z() const {
    return values.get(2, 0);
}

misaxx::misa_quantity<double, misa_ome_unit_length<1>> misa_ome_voxel_size::get_size_xy() const {
    if(get_size_x() == get_size_y())
        return get_size_x();
    else
        throw std::runtime_error("Voxel width and height are assumed to be the same!");
}



