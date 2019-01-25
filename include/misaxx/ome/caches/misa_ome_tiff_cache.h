//
// Created by rgerst on 19.11.18.
//


#pragma once

#include <misaxx/core/misa_cache.h>
#include <misaxx/core/misa_default_cache.h>
#include <ome/files/in/OMETIFFReader.h>
#include <misaxx/ome/patterns/misa_ome_tiff_pattern.h>
#include <misaxx/ome/descriptions/misa_ome_tiff_description.h>
#include <misaxx/core/misa_cached_data.h>
#include <misaxx/ome/utils/ome_tiff_io.h>
#include <misaxx/ome/accessors/misa_ome_plane.h>

namespace misaxx::ome {

    /**
     * Cache that allows read and write access to an OME TIFF
     * @tparam Image cv::Mat or a coixx::image
     */
    class misa_ome_tiff_cache : public misaxx::misa_default_cache<misaxx::utils::memory_cache<std::vector<misa_ome_plane>>,
            misa_ome_tiff_pattern, misa_ome_tiff_description> {
    public:

        void do_link(const misa_ome_tiff_description &t_description) override;

        bool has() const override;

        /**
         * Returns the ome::files tiff reader instance.
         * @return
         */
        std::shared_ptr<ome_tiff_io> get_tiff_io() const;

        /**
         * Returns the plane cache from a plane location
         * @param t_cache
         * @param t_location
         * @return
         */
        misa_ome_plane get_plane(const misa_ome_plane_description &t_location) const;

        void postprocess() override;

    protected:
        misa_ome_tiff_description produce_description(const boost::filesystem::path &t_location,
                                                      const misa_ome_tiff_pattern &t_pattern) override;

    private:

        std::shared_ptr<ome_tiff_io> m_tiff;

    };
}