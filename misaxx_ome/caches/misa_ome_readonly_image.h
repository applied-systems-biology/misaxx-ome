//
// Created by rgerst on 19.11.18.
//


#pragma once
#include <misaxx/misa_cache.h>
#include <cxxh/access/cache.h>
#include <opencv2/opencv.hpp>
#include <ome/files/in/OMETIFFReader.h>
#include <misaxx/misa_default_cache.h>
#include <misaxx_ome/patterns/misa_ome_image_pattern.h>
#include <misaxx_ome/io/ome_to_opencv.h>

namespace misaxx_ome {
    struct misa_ome_readonly_image : public misaxx::misa_default_cache<cxxh::access::cache<cv::Mat>, misa_ome_image_pattern, misa_ome_image_description> {
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
            return static_cast<bool>(m_tiff_reader);
        }

        void pull() override {
            m_tiff_reader->setSeries(m_series); // TODO: Mutex?
            m_cached_image = ome_to_opencv(*m_tiff_reader, m_index);
        }

        void stash() override {
            m_cached_image = cv::Mat();
        }

        void push() override {
            // Readonly image; no pushing allowed
        }

        void do_link(const misa_ome_image_description &t_description) override {
            // Won't do anything, as we depend on the tiff_reader (and internal coordinates)
            if(!static_cast<bool>(m_tiff_reader)) {
                throw std::runtime_error("ome_readonly_image: please run manual_link before do_link!");
            }
            set_unique_location(get_location() / "images" / ("s" + std::to_string(m_series)) / (std::to_string(m_index) + ".tif"));
        }

        void manual_link(std::shared_ptr<ome::files::in::OMETIFFReader> t_tiff_reader, ome::files::dimension_size_type t_series, ome::files::dimension_size_type t_index) {
            std::cout << "[Cache] Linking OME TIFF image @ series " << t_series << ", index " << t_index << std::endl;
            m_tiff_reader = std::move(t_tiff_reader);
            m_series = t_series;
            m_index = t_index;
        }

    protected:
        misa_ome_image_description produce_description(const boost::filesystem::path &t_location,
                                                       const misa_ome_image_pattern &t_pattern) override {
            misa_ome_image_description result;
            return result;
        }

    private:
        ome::files::dimension_size_type m_series;
        ome::files::dimension_size_type m_index;
        std::shared_ptr<ome::files::in::OMETIFFReader> m_tiff_reader;
        cv::Mat m_cached_image;
    };
}
