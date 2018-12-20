//
// Created by rgerst on 19.11.18.
//


#pragma once
#include <misaxx/misa_cache.h>
#include <misaxx/misa_manual_cache.h>
#include <misaxx-helpers/cache.h>
#include <misaxx-helpers/string.h>
#include <opencv2/opencv.hpp>
#include <ome/files/in/OMETIFFReader.h>
#include <misaxx-coixx/toolbox/toolbox_opencv.h>
#include <misaxx/misa_default_cache.h>
#include <misaxx-ome/io/ome_tiff_io.h>
#include <misaxx-ome/descriptions/misa_ome_plane_description.h>

namespace misaxx_ome {

    /**
     * Caches a plane within an OME TIFF file.
     * The plane is accessed via a misa_ome_plane_location that indicates where the 2D image data is located within the TIFF file.
     * @tparam Image cv::Mat or a coixx::image
     */
    template<class Image = cv::Mat> struct misa_ome_plane_cache : public misaxx::misa_manual_cache<cxxh::cache<Image>, misa_ome_plane_description> {
        Image &get() override {
            return m_cached_image;
        }

        const Image &get() const override {
            return m_cached_image;
        }

        void set(Image value) override {
            m_cached_image = std::move(value);
        }

        bool has() const override {
            if constexpr (std::is_same<Image, cv::Mat>::value) {
                return !m_cached_image.empty();
            }
            else {
                return !m_cached_image.is_empty();
            }
        }

        bool can_pull() const override {
            return static_cast<bool>(m_tiff);
        }

        void pull() override {
            if constexpr (std::is_same<Image, cv::Mat>::value) {
                m_cached_image = m_tiff->read_plane(get_plane_location());
            }
            else {
                cv::Mat plane = m_tiff->read_plane(get_plane_location());
                m_cached_image = coixx::toolbox::opencv::semantic_from<Image>(plane);
            }
        }

        void stash() override {
            if constexpr (std::is_same<Image, cv::Mat>::value) {
                m_cached_image.release();
            }
            else {
                m_cached_image = Image();
            }
        }

        void push() override {
            if constexpr (std::is_same<Image, cv::Mat>::value) {
                if (m_cached_image.empty())
                    throw std::runtime_error("Trying to write empty image to TIFF!");
                m_tiff->write_plane(m_cached_image, get_plane_location());
            }
            else {
                if (m_cached_image.is_empty())
                    throw std::runtime_error("Trying to write empty image to TIFF!");
                m_tiff->write_plane(m_cached_image.get_image(), get_plane_location());
            }
        }

        void do_link(const misa_ome_plane_description &t_description) override {
            // Won't do anything, as we depend on the tiff_reader (and internal coordinates)
            if(!static_cast<bool>(m_tiff)) {
                throw std::runtime_error("Cannot link OME TIFF plane without a TIFF IO!");
            }
            this->set_unique_location(this->get_location() / "planes" /  (cxxh::to_string(t_description) + ".tif"));
            std::cout << "[Cache] Linking OME TIFF plane @ " << t_description << std::endl;
        }

        void set_tiff_io(std::shared_ptr<ome_tiff_io> t_tiff) {
            m_tiff = std::move(t_tiff);
        }

        std::shared_ptr<ome_tiff_io> get_tiff_io() const {
            return m_tiff;
        }

        /**
         * Gets the location within the OME TIFF
         * @return
         */
        const misa_ome_plane_description &get_plane_location() const {
            return this->describe()->template get<misa_ome_plane_description>();
        }

    private:
        std::shared_ptr<ome_tiff_io> m_tiff;
        Image m_cached_image;
    };
}
