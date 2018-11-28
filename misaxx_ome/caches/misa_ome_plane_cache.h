//
// Created by rgerst on 19.11.18.
//


#pragma once
#include <misaxx/misa_cache.h>
#include <misaxx/misa_manual_cache.h>
#include <cxxh/cache.h>
#include <cxxh/string.h>
#include <opencv2/opencv.hpp>
#include <ome/files/in/OMETIFFReader.h>
#include <misaxx/misa_default_cache.h>
#include <misaxx_ome/io/ome_to_opencv.h>
#include <misaxx_ome/io/ome_read_write_tiff.h>

namespace misaxx_ome {

    /**
     * Caches a plane within an OME TIFF file.
     * The plane is accessed via a misa_ome_plane_location that indicates where the 2D image data is located within the TIFF file.
     */
    struct misa_ome_plane_cache : public misaxx::misa_manual_cache<cxxh::cache<cv::Mat>, misa_ome_plane_location> {
        cv::Mat &get() override {
            return m_cached_image;
        }

        const cv::Mat &get() const override {
            return m_cached_image;
        }

        void set(cv::Mat value) override {
            m_cached_image = std::move(value);
        }

        bool has() const override {
            return !m_cached_image.empty();
        }

        bool can_pull() const override {
            return static_cast<bool>(m_tiff);
        }

        void pull() override {
            m_cached_image = m_tiff->read_plane(describe()->get<misa_ome_plane_location>());
        }

        void stash() override {
            m_cached_image.release();
        }

        void push() override {
            if(m_cached_image.empty())
                throw std::runtime_error("Trying to write empty image to TIFF!");
            m_tiff->write_plane(m_cached_image, describe()->get<misa_ome_plane_location>());
        }

        void do_link(const misa_ome_plane_location &t_description) override {
            // Won't do anything, as we depend on the tiff_reader (and internal coordinates)
            if(!static_cast<bool>(m_tiff)) {
                throw std::runtime_error("Cannot link OME TIFF plane without a TIFF reader/writer!");
            }
            set_unique_location(get_location() / "images" /  (cxxh::to_string(t_description) + ".tif"));
            std::cout << "[Cache] Linking OME TIFF plane @ " << t_description << std::endl;
        }

        void set_tiff(std::shared_ptr<ome_read_write_tiff> t_tiff) {
            m_tiff = std::move(t_tiff);
        }

    private:
        std::shared_ptr<ome_read_write_tiff> m_tiff;
        cv::Mat m_cached_image;
    };
}
