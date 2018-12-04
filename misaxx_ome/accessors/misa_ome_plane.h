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
     * @tparam Image cv::Mat or a coixx::image
     */
    template<class Image = cv::Mat> struct misa_ome_plane : public misaxx::misa_cached_data<misa_ome_plane_cache<Image>>,
                            public misaxx::misa_description_accessors_from_cache<misa_ome_plane_cache<Image>, misa_ome_plane<Image>> {

        /**
         * Clones the image stored in this OME TIFF plane
         * @return
         */
        Image clone() {
            return this->access_readonly().get().clone();
        }

        /**
         * Writes data into this OME TIFF plane
         * @param t_cache
         * @param t_data
         */
        void write(Image t_data) {
            this->access_write().set(std::move(t_data));
        }

        /**
         * Returns the location of this plane within the TIFF file
         * @return
         */
        const misa_ome_plane_location &get_plane_location() const {
            return this->data->get_plane_location();
        }

        /**
         * Width of this plane
         * @param series
         * @return
         */
        size_t get_size_x() const {
            return this->data->get_tiff_io()->get_size_x(get_plane_location().series);
        }

        /**
         * Height of this plane
         * @param series
         * @return
         */
        size_t get_size_y() const {
            return this->data->get_tiff_io()->get_size_y(get_plane_location().series);
        }

        /**
         * Returns the OME TIFF metadata storage
         * @return
         */
        std::shared_ptr<ome::xml::meta::OMEXMLMetadata> get_ome_metadata() const {
            return this->data->get_tiff_io()->get_metadata();
        }
    };
}




