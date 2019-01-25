//
// Created by rgerst on 19.11.18.
//


#pragma once
#include <misaxx/core/misa_cache.h>
#include <misaxx/core/misa_manual_cache.h>
#include <misaxx/core/utils/string.h>
#include <opencv2/opencv.hpp>
#include <ome/files/in/OMETIFFReader.h>
#include <misaxx/imaging/coixx/toolbox/toolbox_opencv.h>
#include <misaxx/core/misa_default_cache.h>
#include <misaxx/ome/utils/ome_tiff_io.h>
#include <misaxx/ome/descriptions/misa_ome_plane_description.h>

namespace misaxx::ome {

    /**
     * Caches a plane within an OME TIFF file.
     * The plane is accessed via a misa_ome_plane_location that indicates where the 2D image data is located within the TIFF file.
     * @tparam Image cv::Mat or a coixx::image
     */
    struct misa_ome_plane_cache : public misaxx::misa_manual_cache<misaxx::utils::cache<cv::Mat>, misa_ome_plane_description> {
        cv::Mat &get() override;

        const cv::Mat &get() const override;

        void set(cv::Mat value) override;

        bool has() const override;

        bool can_pull() const override;

        void pull() override;

        void stash() override;

        void push() override;

        void do_link(const misa_ome_plane_description &t_description) override;

        void set_tiff_io(std::shared_ptr<ome_tiff_io> t_tiff);

        std::shared_ptr<ome_tiff_io> get_tiff_io() const;

        /**
         * Gets the location within the OME TIFF
         * @return
         */
        const misa_ome_plane_description &get_plane_location() const;

    private:
        std::shared_ptr<ome_tiff_io> m_tiff;
        cv::Mat m_cached_image;
    };
}
