//
// Created by rgerst on 19.11.18.
//


#pragma once

#include <misaxx/misa_cache.h>
#include <misaxx/misa_default_cache.h>
#include <ome/files/in/OMETIFFReader.h>
#include <misaxx_ome/patterns/misa_ome_tiff_pattern.h>
#include <misaxx_ome/descriptions/misa_ome_tiff_description.h>
#include <misaxx/misa_cached_data.h>
#include <misaxx_ome/io/ome_read_write_tiff.h>
#include <misaxx/misa_cached_data.h>
#include "misa_ome_plane.h"

namespace misaxx_ome {

    /**
     * List of planes that are stored within an OME TIFF
     */
    using misa_ome_tiff_planes_t = std::vector<misaxx::misa_cached_data<misa_ome_plane>>;

    /**
     * Cache that allows read and write access to an OME TIFF
     */
    class misa_ome_tiff : public misaxx::misa_default_cache<cxxh::access::memory_cache<misa_ome_tiff_planes_t>,
            misa_ome_tiff_pattern, misa_ome_tiff_description> {
    public:

        void do_link(const misa_ome_tiff_description &t_description) override {
            // We do cache initialization during linkage
            set_unique_location(get_location() / t_description.filename);

            if(boost::filesystem::is_regular_file(get_unique_location())) {
                std::cout << "[Cache] Opening OME TIFF " << get_unique_location() << std::endl;
                m_tiff = std::make_shared<ome_read_write_tiff>(get_unique_location());
                auto &description = describe()->access<misa_ome_tiff_description>();

                std::vector<misaxx::misa_cached_data<misa_ome_plane>> images;

                // Load the data from existing TIFF
                using namespace ome::files;
                for(dimension_size_type i = 0; i < m_tiff->get_num_series(); ++i) {
                    m_tiff->get_reader().value->setSeries(i);
                    description.series.emplace_back(misa_ome_series_description(*m_tiff->get_metadata(), i));

                    // Iterate through all planes in this series
                    const auto end = misa_ome_plane_location::get_end(*(m_tiff->get_reader().value), i);
                    for(auto j = misa_ome_plane_location(i); j != end; j.increment_in_series(end)) {
                        misaxx::misa_cached_data<misa_ome_plane> cache;
                        cache.cache = std::make_shared<misa_ome_plane>();
                        cache.cache->manual_link(m_tiff, i, j);
                        images.emplace_back(std::move(cache));
                    }
                }

                get() = std::move(images);
            }
            else {
                // Generate from the description
                if(t_description.series.empty())
                    throw std::runtime_error("Cannot create a new OME TIFF without any series!");
                using namespace ome::files;

                // Create the native OME metadata
                auto meta = std::make_shared<ome::xml::meta::OMEXMLMetadata>();
                std::vector<std::shared_ptr<CoreMetadata>> series_list;

                for(size_t i = 0; i < t_description.series.size(); ++i) {
                    const auto &series = t_description.series[i];
                    series_list.emplace_back(series.as_ome());
                }

                ome::files::fillMetadata(*meta, series_list);

                // Create the TIFF and generate the image caches
                m_tiff = std::make_shared<ome_read_write_tiff>(get_unique_location(), meta);

                for(size_t i = 0; i < t_description.series.size(); ++i) {
                    const auto &series = t_description.series[i];
                    std::vector<misaxx::misa_cached_data<misa_ome_plane>> images;

                    for(size_t j = 0; j < series.num_images; ++j) {
                        misaxx::misa_cached_data<misa_ome_plane> cache;
                        cache.cache = std::make_shared<misa_ome_plane>();
                        cache.cache->manual_link(m_tiff, i, j);
                        images.emplace_back(std::move(cache));
                    }
                    get().emplace_back(std::move(images));
                }
            }
        }

        bool has() const override {
            return static_cast<bool>(m_tiff);
        }

        /**
         * Returns the ome::files tiff reader instance.
         * @return
         */
        std::shared_ptr<ome_read_write_tiff> get_tiff_reader() const {
            return m_tiff;
        }

    public:

        /**
         * Convenience function that returns a readonly access
         * @param series
         * @param image
         * @return
         */
        static cxxh::access::readonly_access<cv::Mat> readonly_access_image(const misaxx::misa_cached_data<misa_ome_tiff> &t_cache, size_t series, size_t image) {
            return t_cache.access_readonly().get().at(series).at(image).access_readonly();
        }

        /**
         * Convenience function that returns a read & write access
         * @param series
         * @param image
         * @return
         */
        static cxxh::access::readwrite_access<cv::Mat> readwrite_access_image(const misaxx::misa_cached_data<misa_ome_tiff> &t_cache, size_t series, size_t image) {
            return t_cache.access_readonly().get().at(series).at(image).access_readwrite();
        }

        /**
         * Convenience function that writes image data into the OME TIFF
         * @param t_cache
         * @param data
         * @param series
         * @param image
         */
        static void write_image(const misaxx::misa_cached_data<misa_ome_tiff> &t_cache, cv::Mat data, size_t series, size_t image) {
            auto access = t_cache.access_readonly().get().at(series).at(image).access_write();
            access.set(std::move(data));
        }

        /**
         * Convenience function that reads a copy of image data from an OME TIFF
         * @param t_cache
         * @param data
         * @param series
         * @param image
         * @return Copy of cached image
         */
        static cv::Mat read_image(const misaxx::misa_cached_data<misa_ome_tiff> &t_cache, size_t series, size_t image) {
            auto access = t_cache.access_readonly().get().at(series).at(image).access_readonly();
            return access.get().clone();
        }

        /**
         * Convenience function that returns the numer of series in the OME TIFF
         * @param t_cache
         * @return
         */
        static size_t get_num_series(const misaxx::misa_cached_data<misa_ome_tiff> &t_cache) {
            return t_cache.access_readonly().get().size();
        }

        /**
         * Convenience function that returns the number of images in an image series
         * @param t_cache
         * @param series
         * @return
         */
        static size_t get_num_images(const misaxx::misa_cached_data<misa_ome_tiff> &t_cache, size_t series) {
            return t_cache.access_readonly().get().at(series).size();
        }

    protected:
        misa_ome_tiff_description produce_description(const boost::filesystem::path &t_location,
                                                      const misa_ome_tiff_pattern &t_pattern) override {
            auto file_description = t_pattern.produce(t_location);
            misa_ome_tiff_description result;
            result.filename = file_description.filename;
            return result;
        }

    private:
        std::shared_ptr<ome_read_write_tiff> m_tiff;

        /**
         * Maps the series index to the start index within the MISA OME TIFF wrapper
         */
        std::unordered_map<size_t, size_t> m_series_start_indices;
    };
}