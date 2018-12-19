//
// Created by rgerst on 28.11.18.
//

#pragma once

#include <misaxx-ome/caches/misa_ome_tiff_cache.h>
#include <misaxx/misa_cached_data.h>
#include <misaxx-ome/misa_ome_tiff_description_builder.h>
#include <misaxx-ome/misa_ome_tiff_description_modifier.h>

namespace misaxx_ome {
    /**
     * An OME TIFF file that contains a list of 2D image planes.
     * The planes can be accessed via their index within this structure or using a misa_ome_plane_location
     * that also encodes semantic location within a time/depth/channel space.
     * @tparam Image cv::Mat or a coixx::image
     */
    template<class Image = cv::Mat> struct misa_ome_tiff : public misaxx::misa_cached_data<misa_ome_tiff_cache<Image>>,
                           public misaxx::misa_description_accessors_from_cache<misa_ome_tiff_cache<Image>, misa_ome_tiff<Image>> {

        using iterator = typename std::vector<misa_ome_plane<Image>>::iterator;
        using const_iterator = typename std::vector<misa_ome_plane<Image>>::const_iterator;

        iterator begin() {
            return this->data->get().begin();
        }

        iterator end() {
            return this->data->get().end();
        }

        const_iterator begin() const {
            return this->data->get().begin();
        }

        const_iterator end() const {
            return this->data->get().end();
        }

        size_t size() const {
            return this->data->get().size();
        }

        bool empty() const {
            return this->data->get().empty();
        }

        misa_ome_plane<Image> at(size_t index) const {
            return this->data->get().at(index);
        }

        misa_ome_plane<Image> at(const misa_ome_plane_description &index) {
            return this->data->get_plane(index);
        }

        /**
         * Width of each plane in the TIFF
         * @param series
         * @return
         */
        size_t get_size_x(size_t series = 0) const {
            return this->data->get_tiff_io()->get_size_x(series);
        }

        /**
         * Height of each plane in the TIFF
         * @param series
         * @return
         */
        size_t get_size_y(size_t series = 0) const {
            return this->data->get_tiff_io()->get_size_y(series);
        }

        /**
         * Number of planes allocated within the time axis
         * @param series
         * @return
         */
        size_t get_size_t(size_t series = 0) const {
            return this->data->get_tiff_io()->get_size_t(series);
        }

        /**
         * Number of planes allocated within the channel axis
         * This is not equal to the number of channels each pixel consists of.
         * @param series
         * @return
         */
        size_t get_size_c(size_t series = 0) const {
            return this->data->get_tiff_io()->get_size_c(series);
        }

        /**
         * Number of planes allocated within the depth axis
         * @param series
         * @return
         */
        size_t get_size_z(size_t series = 0) const {
            return this->data->get_tiff_io()->get_size_z(series);
        }

        /**
         * Returns a description builder that allows changing properties of the description more easily
         * The description builder works on a copy
         * @return
         */
        misa_ome_tiff_description_modifier derive() const {
            return misa_ome_tiff_description_modifier(this->get_data_description());
        }

        /**
         * Returns the OME TIFF metadata storage
         * @return
         */
        std::shared_ptr<ome::xml::meta::OMEXMLMetadata> get_ome_metadata() const {
            return this->data->get_tiff_io()->get_metadata();
        }

        /**
         * Creates a builder that allows creating a TIFF description from scratch
         * Please note that if a source description is provided, additional metadata is not copied.
         * Use derive() instead in this case
         * @param src Optional source description
         * @return
         */
        static misa_ome_tiff_description_builder build(misa_ome_tiff_description src = misa_ome_tiff_description("image.ome.tif")) {
            return misa_ome_tiff_description_builder(std::move(src));
        }
    };
}




