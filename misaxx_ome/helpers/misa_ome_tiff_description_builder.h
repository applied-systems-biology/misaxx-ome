//
// Created by rgerst on 03.12.18.
//


#pragma once

#include <memory>
#include <misaxx_ome/descriptions/misa_ome_tiff_description.h>
#include <opencv2/opencv.hpp>
#include <misaxx_ome/io/opencv_to_ome.h>
#include <misaxx_ome/io/coixx_to_ome.h>
#include <misaxx/misa_description_storage.h>
#include <misaxx_ome/helpers/ome_helpers.h>

namespace misaxx_ome {

    /**
     * Builder that allows easy creation of OME XML Metadata from scratch.
     */
    struct misa_ome_tiff_description_builder {

        misa_ome_tiff_description_builder() = default;

        /**
         * Returns the final result as OMEXMLMetadata
         * @return
         */
        std::shared_ptr<ome::xml::meta::OMEXMLMetadata> as_ome_metadata() {
            auto meta = std::make_shared<ome::xml::meta::OMEXMLMetadata>();
            ome::files::fillMetadata(*meta, series_list);
            return meta;
        }

        /**
         * Returns the final TIFF description
         * @return
         */
        misa_ome_tiff_description as_description() {
            misa_ome_tiff_description result;
            result.metadata = as_ome_metadata();
            return result;
        }

        /**
         * Returns the final TIFF description as storage
         * @return
         */
        std::shared_ptr<misaxx::misa_description_storage> as_storage() {
            return misaxx::misa_description_storage::with(as_description());
        }

        /**
         * All consecutive edit operations run on the provided series.
         * Will create series if necessary
         * @param series
         * @return
         */
        misa_ome_tiff_description_builder &change_series(size_t series) {
            if (series < 0)
                throw std::runtime_error("The series must be at least 0");
            m_series = series;

            while(series_list.size() <= series) {
                series_list.emplace_back(std::make_shared<ome::files::CoreMetadata>());
            }

            return *this;
        }

        ome::files::CoreMetadata &core_metadata() {
            change_series(m_series);
            return *series_list.at(m_series);
        }

        /**
         * Sets the type each channel in a plane has
         * @param t_pixel_type
         * @return
         */
        misa_ome_tiff_description_builder &pixel_channel_type(const ome::xml::model::enums::PixelType &t_pixel_type) {
            core_metadata().pixelType = t_pixel_type;
            core_metadata().bitsPerPixel = helpers::ome_pixel_type_to_bits_per_pixel(t_pixel_type);
            return *this;
        }

        /**
         * Sets the number of channels each pixels has
         * @param channels
         * @return
         */
        misa_ome_tiff_description_builder &pixel_channels(std::vector<size_t> channels) {
            if(core_metadata().sizeC.empty())
                throw std::runtime_error("Please set the number of channels in the channel axis first!");
            if (channels.size() == 1) {
                for (auto &s : core_metadata().sizeC) {
                    s = channels.at(0);
                }
            } else {
                if (channels.size() != core_metadata().sizeC.size())
                    throw std::runtime_error(
                            "The size of the provided channel configuration should match the number of planes in channel axis! Please set the number of channels in the channel axis first!");
                core_metadata().sizeC = std::move(channels);
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
            std::vector<size_t> c = {channels};
            return pixel_channels(std::move(c));
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
        template<class Image>
        misa_ome_tiff_description_builder &of_coixx() {
            pixel_channels(Image::color_type::channels);
            return pixel_channel_type(coixx_to_ome_pixel_type<Image>());
        }

        /**
         * Sets the number of planes in the Z (depth) axis
         * @param size
         * @return
         */
        misa_ome_tiff_description_builder &depth(size_t size) {
            core_metadata().sizeZ = size;
            return *this;
        }

        /**
         * Sets the number of planes in the channel axis
         * This is not the number of channels each pixel has
         * @param size
         * @return
         */
        misa_ome_tiff_description_builder &channels(size_t size, size_t num_channels = 1) {
            if(core_metadata().sizeC.size() < size) {
                while(core_metadata().sizeC.size() <= size) {
                    core_metadata().sizeC.push_back(num_channels);
                }
            }
            else if(core_metadata().sizeC.size() > size) {
                core_metadata().sizeC.resize(size); // Remove the last
            }
            return *this;
        }

        /**
         * Sets the number of planes in the time axis
         * @param size
         * @return
         */
        misa_ome_tiff_description_builder &duration(size_t size) {
            core_metadata().sizeT = size;
            return *this;
        }

        /**
         * Sets the width of the planes
         * @param size
         * @return
         */
        misa_ome_tiff_description_builder &width(size_t size) {
            core_metadata().sizeX = size;
            return *this;
        }

        /**
         * Sets the height of the planes
         * @param size
         * @return
         */
        misa_ome_tiff_description_builder &height(size_t size) {
            core_metadata().sizeY = size;
            return *this;
        }

        /**
         * Sets the width and height
         * @param w
         * @param h
         * @return
         */
        misa_ome_tiff_description_builder &of_size(size_t w, size_t h) {
            return width(w).height(h);
        }

        /**
         * Modifies the description with a custom function
         * @tparam Function
         * @param t_function Function that takes a std::vector<std::shared_ptr<ome::files::CoreMetadata>>
         * @return
         */
        template<class Function>
        misa_ome_tiff_description_builder &modify(const Function &t_function) {
            t_function(series_list);
            return *this;
        }

    private:
        /**
         * The list of series that is managed by this description builder
         */
        std::vector<std::shared_ptr<ome::files::CoreMetadata>> series_list;
        /**
         * The current series
         */
        size_t m_series = 0;
    };

}