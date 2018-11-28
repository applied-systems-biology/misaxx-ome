//
// Created by rgerst on 28.11.18.
//

#pragma once

#include <misaxx/misa_cached_data.h>
#include <misaxx_ome/caches/misa_ome_plane_cache.h>
#include <misaxx/misa_default_description_accessors.h>

namespace misaxx_ome {
    /**
     * A 2D image (plane) stored inside an OME TIFF file.
     */
    struct misa_ome_plane : public misaxx::misa_cached_data<misa_ome_plane_cache>,
                            public misaxx::misa_description_accessors_from_cache<misa_ome_plane_cache, misa_ome_plane> {

        /**
         * Clones the image stored in this OME TIFF plane
         * @return
         */
        cv::Mat clone() {
            return this->access_readonly().get().clone();
        }

        /**
         * Writes data into this OME TIFF plane
         * @param t_cache
         * @param t_data
         */
        void write(cv::Mat t_data) {
            this->access_write().set(std::move(t_data));
        }
    };
}




