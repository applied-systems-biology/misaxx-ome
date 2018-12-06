//
// Created by rgerst on 05.12.18.
//

#pragma once


#include <misaxx/attachments/misa_voxel.h>
#include <misaxx_ome/attachments/misa_ome_quantity.h>

namespace misaxx_ome {
    /**
     * Helper function that creates a misa_voxel<misa_ome_length<double>> instance
     * @param x
     * @param y
     * @param z
     * @param unit
     * @return
     */
    inline misaxx::misa_voxel<misa_ome_length<double>> misa_ome_voxel(double x, double y, double z, const misa_ome_unit_length &unit) {
        auto zero = misa_ome_length<double>(0, unit);
        auto x_ = misa_ome_length<double>(x, unit);
        auto y_ = misa_ome_length<double>(y, unit);
        auto z_ = misa_ome_length<double>(z, unit);
        return misaxx::misa_voxel<misa_ome_length<double>>({ zero,  std::move(x_)}, {zero, std::move(y_)}, {zero, std::move(z_)});
    }
}


