//
// Created by rgerst on 13.12.18.
//


#pragma once

#include <misaxx/attachments/misa_matrix.h>
#include "misa_ome_unit.h"

namespace misaxx_ome {
    /**
     * Convenience type for a 3d voxel size
     * containing size in X, size in Y and size in Z direction
     */
    using misa_ome_voxel_size = misaxx::misa_vector<double, misaxx_ome::misa_ome_unit_length <1>, 3>;
}