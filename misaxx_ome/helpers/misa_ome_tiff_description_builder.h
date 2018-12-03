//
// Created by rgerst on 03.12.18.
//


#pragma once

#include <memory>
#include <misaxx_ome/descriptions/misa_ome_tiff_description.h>
#include <opencv2/opencv.hpp>
#include <misaxx_ome/io/opencv_to_ome.h>
#include <misaxx_ome/io/coixx_to_ome.h>

namespace misaxx_ome {

    /**
     * Interface that allows easy modification and creation of TIFF descriptions
     */
    struct misa_ome_tiff_description_builder {

        misa_ome_tiff_description_builder() : m_data(std::make_shared<misa_ome_tiff_description>()) {

        }

        explicit misa_ome_tiff_description_builder(std::shared_ptr<misa_ome_tiff_description> t_data) : m_data(std::move(t_data)) {

        }

        /**
         * Returns the final TIFF description
         * @return
         */
        std::shared_ptr<misa_ome_tiff_description> as_description() {
            return m_data;
        }

        /**
         * Returns the final TIFF description as storage
         * @return
         */
        std::shared_ptr<misaxx::misa_description_storage> as_storage() {
            return misaxx::misa_description_storage::with(*m_data);
        }

        /**
         * All consecutive edit operations run on the provided series.
         * Will create series if necessary
         * @param series
         * @return
         */
        misa_ome_tiff_description_builder &series(size_t series) {
            if(series < 0)
                throw std::runtime_error("The series must be at least 0");
            m_series = series;

            if(m_data->series.empty()) {
                for(size_t i = 0; i <= series; ++series) {
                    m_data->series.emplace_back(misa_ome_series_description());
                }
            }
            else {
                for(size_t i = m_data->series.size() - 1; i <= series; ++series) {
                    m_data->series.emplace_back(misa_ome_series_description());
                }
            }

            return *this;
        }

       /**
        * Sets the type each channel in a plane has
        * @param t_pixel_type
        * @return
        */
        misa_ome_tiff_description_builder &pixel_channel_type(const ome::xml::model::enums::PixelType &t_pixel_type) {
            series(m_series); //ensure current series
            m_data->series.at(m_series).channel_type = t_pixel_type;
            return *this;
        }

        /**
         * Sets the number of channels each pixels has
         * @param channels
         * @return
         */
        misa_ome_tiff_description_builder &pixel_channels(std::vector<size_t> channels) {
            series(m_series); //ensure current series
            if(channels.size() == 1) {
                for(auto &s : m_data->series.at(m_series).channels) {
                    s = channels.at(0);
                }
            }
            else {
                if(channels.size() != m_data->series.at(m_series).channels.size())
                    throw std::runtime_error("The size of the provided channel configuration should match the number of planes in channel axis!");
                m_data->series.at(m_series).channels = std::move(channels);
            }

            return *this;
        }

        /**
         * Sets the number of channels each pixels has
         * Requires an already set number of planes in channel axis
         * @param channels
         * @return
         */
        misa_ome_tiff_description_builder &pixel_channels(size_t channels) {
            std::vector<size_t> c = { channels };
            return pixel_channels (std::move(c));
        }

        /**
         * Sets the number of channels and the pixel type
         * @param channels
         * @param t_pixel_type
         * @return
         */
        misa_ome_tiff_description_builder &of(size_t channels, const ome::xml::model::enums::PixelType &t_pixel_type) {
            pixel_channels(channels);
            return pixel_channel_type(t_pixel_type);
        }

        /**
         * Sets the number of channels and the pixel type from an OpenCV type
         * @param opencv_type
         * @return
         */
        misa_ome_tiff_description_builder &of_opencv(int opencv_type) {
            cv::Mat m(1, 1, opencv_type);
            pixel_channels(m.channels());
            return pixel_channel_type(opencv_depth_to_ome_pixel_type(m.depth()));
        }

        /**
         * Sets the number of channels and the pixel type from an coi++ image type
         * @param opencv_type
         * @return
         */
        template<class Image> misa_ome_tiff_description_builder &of_coixx() {
            pixel_channels(Image::color_type::channels);
            return pixel_channel_type(coixx_to_ome_pixel_type<Image>());
        }

        /**
         * Sets the number of planes in the Z (depth) axis
         * @param size
         * @return
         */
        misa_ome_tiff_description_builder &depth(size_t size) {
            series(m_series); //ensure current series
            m_data->series.at(m_series).size_Z = size;
            return *this;
        }

        /**
         * Sets the number of planes in the channel axis
         * This is not the number of channels each pixel has
         * @param size
         * @return
         */
        misa_ome_tiff_description_builder &channels(size_t size) {
            series(m_series); //ensure current series
            m_data->series.at(m_series).size_Z = size;
            return *this;
        }

        /**
         * Sets the number of planes in the time axis
         * @param size
         * @return
         */
        misa_ome_tiff_description_builder &duration(size_t size) {
            series(m_series); //ensure current series
            m_data->series.at(m_series).size_T = size;
            return *this;
        }

        /**
         * Sets the width of the planes
         * @param size
         * @return
         */
        misa_ome_tiff_description_builder &width(size_t size) {
            series(m_series); //ensure current series
            m_data->series.at(m_series).size_X = size;
            return *this;
        }

        /**
         * Sets the height of the planes
         * @param size
         * @return
         */
        misa_ome_tiff_description_builder &height(size_t size) {
            series(m_series); //ensure current series
            m_data->series.at(m_series).size_Y = size;
            return *this;
        }

        /**
         * Sets the width and height
         * @param w
         * @param h
         * @return
         */
        misa_ome_tiff_description_builder &of_size(size_t w, size_t h) {
            width(w);
            return height(h);
        }

        /**
         * Modifies the description with a custom function
         * @tparam Function
         * @param t_function Function that takes a misa_ome_tiff_description reference
         * @return
         */
        template<class Function> misa_ome_tiff_description_builder &modify(const Function &t_function) {
            t_function(*m_data);
            return *this;
        }

    private:
        std::shared_ptr<misa_ome_tiff_description> m_data;
        size_t m_series = 0;
    };

}