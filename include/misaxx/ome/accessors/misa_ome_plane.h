//
// Created by rgerst on 28.11.18.
//

#pragma once

#include <misaxx/core/misa_cached_data.h>
#include <misaxx/ome/caches/misa_ome_plane_cache.h>
#include <misaxx/core/misa_default_description_accessors.h>

namespace misaxx::ome {
    /**
     * A 2D image (plane) stored inside an OME TIFF file.
     */
    struct misa_ome_plane : public misaxx::misa_cached_data<misa_ome_plane_cache>,
                            public misaxx::misa_description_accessors_from_cache<misa_ome_plane_cache, misa_ome_plane> {

        /**
         * Clones the image stored in this OME TIFF plane
         * @return
         */
        cv::Mat clone() const;

        /**
         * Writes data into this OME TIFF plane
         * @param t_cache
         * @param t_data
         */
        void write(cv::Mat t_data);

        /**
         * Returns the location of this plane within the TIFF file
         * @return
         */
        const misa_ome_plane_description &get_plane_location() const;

        /**
         * Width of this plane
         * @param series
         * @return
         */
        size_t get_size_x() const;

        /**
         * Height of this plane
         * @param series
         * @return
         */
        size_t get_size_y() const;

        /**
         * Returns the OME TIFF metadata storage
         * @return
         */
        std::shared_ptr<::ome::xml::meta::OMEXMLMetadata> get_ome_metadata() const;
    };
}




