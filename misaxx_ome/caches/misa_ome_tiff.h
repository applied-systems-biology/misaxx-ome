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
#include "misa_ome_image.h"

namespace misaxx_ome {

    using misa_ome_readonly_tiff_images = std::vector<std::vector<misaxx::misa_cached_data<misa_ome_image>>>;

    /**
     * Cache that allows read and write access to an OME TIFF
     */
    class misa_ome_tiff : public misaxx::misa_default_cache<cxxh::access::memory_cache<misa_ome_readonly_tiff_images>,
            misa_ome_tiff_pattern, misa_ome_tiff_description> {
    public:

        void do_link(const misa_ome_tiff_description &t_description) override {
            // We do cache initialization during linkage
            set_unique_location(get_location() / t_description.filename);

            if(boost::filesystem::is_regular_file(get_unique_location())) {
                std::cout << "[Cache] Opening OME TIFF " << get_unique_location() << std::endl;
                m_tiff = std::make_shared<ome_read_write_tiff>(get_unique_location());

                // Load the data from existing TIFF
                using namespace ome::files;
                for(dimension_size_type i = 0; i < m_tiff->get_num_series(); ++i) {
                    std::vector<misaxx::misa_cached_data<misa_ome_image>> images;

                    for(dimension_size_type j = 0; j < m_tiff->get_num_images(i); ++j) {
                        misaxx::misa_cached_data<misa_ome_image> cache;
                        cache.cache = std::make_shared<misa_ome_image>();
                        cache.cache->manual_link(m_tiff, i, j);
                        images.emplace_back(std::move(cache));
                    }
                    get().emplace_back(std::move(images));
                }
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
                    std::vector<misaxx::misa_cached_data<misa_ome_image>> images;

                    for(size_t j = 0; j < series.num_images; ++j) {
                        misaxx::misa_cached_data<misa_ome_image> cache;
                        cache.cache = std::make_shared<misa_ome_image>();
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
    };
}