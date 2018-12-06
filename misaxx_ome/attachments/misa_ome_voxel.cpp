//
// Created by rgerst on 05.12.18.
//

#include "misa_ome_voxel.h"

misaxx::misa_voxel<misaxx_ome::misa_ome_length<double>>
misaxx_ome::misa_ome_voxel(double x, double y, double z, const misaxx_ome::misa_ome_unit_length &unit) {
    auto zero = misa_ome_length<double>(0, unit);
    auto x_ = misa_ome_length<double>(x, unit);
    auto y_ = misa_ome_length<double>(y, unit);
    auto z_ = misa_ome_length<double>(z, unit);
    return misaxx::misa_voxel<misa_ome_length<double>>({zero, std::move(x_)}, {zero, std::move(y_)},
                                                       {zero, std::move(z_)});
}

misaxx::misa_voxel<misaxx_ome::misa_ome_length<double>>
misaxx_ome::misa_ome_voxel(misaxx::misa_voxel<misaxx_ome::misa_ome_length<double>> v,
                           const misaxx_ome::misa_ome_unit_length &unit) {
    v.range_x.from.convert_to(unit);
    v.range_x.to.convert_to(unit);
    v.range_y.from.convert_to(unit);
    v.range_y.to.convert_to(unit);
    v.range_z.from.convert_to(unit);
    v.range_z.to.convert_to(unit);
    return v;
}

misaxx::misa_voxel<misaxx_ome::misa_ome_length<double>>
misaxx_ome::misa_ome_voxel(const ome::xml::meta::OMEXMLMetadata &meta, size_t series,
                           const std::optional<misaxx_ome::misa_ome_unit_length> &unit) {
    using namespace ome::xml::model::primitives;
    double vx = meta.getPixelsPhysicalSizeX(series).getValue();
    double vy = meta.getPixelsPhysicalSizeY(series).getValue();
    double vz = meta.getPixelsPhysicalSizeZ(series).getValue();
    misa_ome_unit_length ux = meta.getPixelsPhysicalSizeX(series).getUnit();
    misa_ome_unit_length uy = meta.getPixelsPhysicalSizeY(series).getUnit();
    misa_ome_unit_length uz = meta.getPixelsPhysicalSizeZ(series).getUnit();
    if (unit) {
        if (ux!= *unit) {
            vx = convert(Quantity<misa_ome_unit_length, double >(vx, ux), *unit).getValue();
            ux = *unit;
        }
        if (uy!= *unit) {
            vy = convert(Quantity<misa_ome_unit_length, double >(vy, uy), *unit).getValue();
            uy = *unit;
        }
        if (uz!= *unit) {
            vz = convert(Quantity<misa_ome_unit_length, double >(vz, uz), *unit).getValue();
            uz = *unit;
        }
    }

    misaxx::misa_range<misa_ome_length<double>> rx(misa_ome_length<double>(0, ux),
                                                   misa_ome_length<double>(vx, ux));
    misaxx::misa_range<misa_ome_length<double>> ry(misa_ome_length<double>(0, uy),
                                                   misa_ome_length<double>(vy, uy));
    misaxx::misa_range<misa_ome_length<double>> rz(misa_ome_length<double>(0, uz),
                                                   misa_ome_length<double>(vz, uz));

    return misaxx::misa_voxel<misa_ome_length<double>>(rx, ry, rz);
}
