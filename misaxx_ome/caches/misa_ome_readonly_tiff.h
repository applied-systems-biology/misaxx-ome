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
#include "misa_ome_readonly_image.h"

namespace misaxx_ome {

    using misa_ome_readonly_tiff_images = std::vector<std::vector<misaxx::misa_cached_data<misa_ome_readonly_image>>>;

    class misa_ome_readonly_tiff : public misaxx::misa_default_cache<cxxh::access::memory_cache<misa_ome_readonly_tiff_images>,
            misa_ome_tiff_pattern, misa_ome_tiff_description> {
    public:

        void do_link(const misa_ome_tiff_description &t_description) override {
            // We do cache initialization during linkage
            std::cout << "[Cache] Opening OME TIFF " << get_location() / t_description.filename << std::endl;
            m_tiff_reader = std::make_shared<ome::files::in::OMETIFFReader>();
            m_tiff_reader->setMetadataFiltered(false);
            m_tiff_reader->setGroupFiles(true);
            m_tiff_reader->setId(get_location() / t_description.filename);
            set_unique_location(get_location() / t_description.filename);

            // Initialize the tiff image caches
            using namespace ome::files;
            for(dimension_size_type i = 0; i < m_tiff_reader->getSeriesCount(); ++i) {
                m_tiff_reader->setSeries(i);
                std::vector<misaxx::misa_cached_data<misa_ome_readonly_image>> images;
                for(dimension_size_type j = 0; j < m_tiff_reader->getImageCount(); ++j) {
                    misaxx::misa_cached_data<misa_ome_readonly_image> cache;
                    cache.cache = std::make_shared<misa_ome_readonly_image>();
                    cache.cache->manual_link(m_tiff_reader, i, j);
                    images.emplace_back(std::move(cache));
                }
                get().emplace_back(std::move(images));
            }
        }

        bool has() const override {
            return static_cast<bool>(m_tiff_reader);
        }

        /**
         * Returns the ome::files tiff reader instance.
         * @return
         */
        std::shared_ptr<ome::files::in::OMETIFFReader> get_tiff_reader() const {
            return m_tiff_reader;
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
        std::shared_ptr<ome::files::in::OMETIFFReader> m_tiff_reader;
    };
}