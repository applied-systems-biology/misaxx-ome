//
// Created by rgerst on 19.11.18.
//


#pragma once
#include <misaxx/misa_cache.h>
#include <misaxx/misa_manual_cache.h>
#include <cxxh/access/cache.h>
#include <opencv2/opencv.hpp>
#include <ome/files/in/OMETIFFReader.h>
#include <misaxx/misa_default_cache.h>
#include <misaxx_ome/io/ome_to_opencv.h>
#include <misaxx_ome/io/ome_read_write_tiff.h>

namespace misaxx_ome {
struct misa_ome_plane : public misaxx::misa_manual_cache<cxxh::access::cache<cv::Mat>, misa_ome_plane_location> {
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
            m_cached_image = m_tiff->read_plane(m_series, m_index);
        }

        void stash() override {
            m_cached_image.release();
        }

        void push() override {
            if(m_cached_image.empty())
                throw std::runtime_error("Trying to write empty image to TIFF!");
            m_tiff->write_plane(m_cached_image, m_series, m_index);
        }

        void do_link(const misa_ome_plane_description &t_description) override {
            // Won't do anything, as we depend on the tiff_reader (and internal coordinates)
            if(!static_cast<bool>(m_tiff)) {
                throw std::runtime_error("ome_readonly_image: please run manual_link before do_link!");
            }
            set_unique_location(get_location() / "images" / ("s" + std::to_string(m_series)) / (std::to_string(m_index) + ".tif"));
        }

        void manual_link(std::shared_ptr<ome_read_write_tiff> t_tiff, ome::files::dimension_size_type t_series, ome::files::dimension_size_type t_index) {
            std::cout << "[Cache] Linking OME TIFF image @ series " << t_series << ", index " << t_index << std::endl;
            m_tiff = std::move(t_tiff);
            m_series = t_series;
            m_index = t_index;
        }

    private:
        ome::files::dimension_size_type m_series;
        ome::files::dimension_size_type m_index;
        std::shared_ptr<ome_read_write_tiff> m_tiff;
        cv::Mat m_cached_image;
    };
}
