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
#include <misaxx_ome/io/ome_tiff_io.h>
#include <misaxx_ome/accessors/misa_ome_plane.h>

namespace misaxx_ome {

    /**
     * List of planes that are stored within an OME TIFF
     */
    using misa_ome_tiff_planes_t = std::vector<misa_ome_plane>;

    /**
     * Cache that allows read and write access to an OME TIFF
     */
    class misa_ome_tiff_cache : public misaxx::misa_default_cache<cxxh::memory_cache<misa_ome_tiff_planes_t>,
            misa_ome_tiff_pattern, misa_ome_tiff_description> {
    public:

        void do_link(const misa_ome_tiff_description &t_description) override {
            // We do cache initialization during linkage
            set_unique_location(get_location() / t_description.filename);

            bool write_description;

            if(boost::filesystem::is_regular_file(get_unique_location())) {
                std::cout << "[Cache] Opening OME TIFF " << get_unique_location() << std::endl;
                m_tiff = std::make_shared<ome_tiff_io>(get_unique_location());
                write_description = true;
            }
            else {
                std::cout << "[Cache] Creating OME TIFF " << get_unique_location() << std::endl;

                // Generate from the description
                if(t_description.series.empty())
                    throw std::runtime_error("Cannot create a new OME TIFF without any series!");
                using namespace ome::files;

                // Create the native OME metadata
                auto meta = std::make_shared<ome::xml::meta::OMEXMLMetadata>();
                std::vector<std::shared_ptr<CoreMetadata>> series_list;

                for (const auto &series : t_description.series) {
                    series_list.emplace_back(series.as_ome());
                }

                ome::files::fillMetadata(*meta, series_list);

                // TODO test
                for(size_t p = 0; p < t_description.series[0].size_Z; ++p) {
                    meta->setUUIDFileName("img" + cxxh::to_string(p), 0, p);
                }

                // Create the TIFF and generate the image caches
                m_tiff = std::make_shared<ome_tiff_io>(get_unique_location(), meta);
                write_description = false;
            }

            // Create the plane caches
            auto &description = describe()->access<misa_ome_tiff_description>();
            for(size_t series = 0; series < m_tiff->get_num_series(); ++series) {
                // For loaded files, write the series into the description
                if(write_description)
                    description.series.emplace_back(misa_ome_series_description(*m_tiff->get_metadata(), series));

                const auto size_Z = m_tiff->get_size_z(series);
                const auto size_C = m_tiff->get_size_c(series);
                const auto size_T = m_tiff->get_size_t(series);

                for(size_t z = 0; z < size_Z; ++z) {
                    for(size_t c = 0; c < size_C; ++c) {
                        for(size_t t = 0; t < size_T; ++t) {
                            misa_ome_plane cache;
                            cache.data = std::make_shared<misa_ome_plane_cache>();
                            cache.data->set_tiff(m_tiff);
                            cache.force_link(get_location(), misaxx::misa_description_storage::with(misa_ome_plane_location(series, z, c, t)));
                            get().emplace_back(std::move(cache));
                        }
                    }
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
        std::shared_ptr<ome_tiff_io> get_tiff_io() const {
            return m_tiff;
        }

        /**
         * Returns the plane cache from a plance location
         * @param t_cache
         * @param t_location
         * @return
         */
        misa_ome_plane get_plane(const misa_ome_plane_location &t_location) const {
            const auto num_series = m_tiff->get_num_series();
            const auto size_Z = m_tiff->get_size_z(t_location.series);
            const auto size_C = m_tiff->get_size_c(t_location.series);
            const auto size_T = m_tiff->get_size_t(t_location.series);

            // Calculate the plane index
            size_t start_index = 0;
            for(size_t series = 0; series < num_series; ++series) {
                start_index += m_tiff->get_num_planes(series);
            }

            size_t index = start_index + t_location.t + t_location.c * size_T + t_location.z * size_T * size_C;
            return get().at(index);
        }

        void postprocess() override {
            misa_cache::postprocess();
            // Close the TIFF
            m_tiff->close();
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

        std::shared_ptr<ome_tiff_io> m_tiff;

    };
}