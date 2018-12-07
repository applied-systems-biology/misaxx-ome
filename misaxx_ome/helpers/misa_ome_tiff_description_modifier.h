//
// Created by rgerst on 07.12.18.
//

#pragma once

#include <ome/xml/meta/OMEXMLMetadata.h>
#include <misaxx_ome/descriptions/misa_ome_tiff_description.h>
#include <misaxx/misa_description_storage.h>
#include <misaxx_ome/io/coixx_to_ome.h>
#include <misaxx_ome/io/opencv_to_ome.h>
#include <misaxx_ome/helpers/ome_helpers.h>
#include <opencv2/core/mat.hpp>

namespace misaxx_ome {

    /**
     * Builder-like helper that allows creation of a new misa_ome_tiff_description based on
     * an existing one. Additional metadata is kept.
     */
    struct misa_ome_tiff_description_modifier {

        misa_ome_tiff_description_modifier() = default;

        explicit misa_ome_tiff_description_modifier(misa_ome_tiff_description src) : m_result(std::move(src)) {
            // Copy the metadata to allow derivation
            m_result.metadata = ome::files::createOMEXMLMetadata(m_result.metadata->dumpXML());

            // Import the channel configuration
            for(size_t series = 0; series < m_result.metadata->getImageCount(); ++series) {
                std::vector<size_t> channels;

                for(size_t c = 0; c < m_result.metadata->getChannelCount(series); ++c) {
                    channels.push_back(m_result.metadata->getChannelSamplesPerPixel(series, c));
                }

                m_channels.emplace_back(std::move(channels));
            }
        }

        /**
         * Returns the final TIFF description
         * @return
         */
        operator misa_ome_tiff_description() {
            return m_result;
        }

        /**
         * Returns the final TIFF description as storage
         * @return
         */
        operator std::shared_ptr<misaxx::misa_description_storage>() {
            return misaxx::misa_description_storage::with(static_cast<misa_ome_tiff_description>(*this));
        }

        /**
       * All consecutive edit operations run on the provided series.
       * Will create series if necessary
       * @param series
       * @return
       */
        misa_ome_tiff_description_modifier &change_series(size_t series) {
            if (series < 0)
                throw std::runtime_error("The series must be at least 0");
            if(series >= m_result.metadata->getImageCount())
                throw std::runtime_error("The requested series does not exist");
            m_series = series;
            while(m_channels.size() < series + 1) {
                m_channels.emplace_back(std::vector<size_t>());
            }
            return *this;
        }

        /**
        * Sets the type each channel in a plane has
        * @param t_pixel_type
        * @return
        */
        misa_ome_tiff_description_modifier &pixel_channel_type(const ome::xml::model::enums::PixelType &t_pixel_type) {
            change_series(m_series);
            m_result.metadata->setPixelsType(t_pixel_type, m_series);
            m_result.metadata->setPixelsSignificantBits(ome::files::bitsPerPixel(t_pixel_type), m_series);
            return *this;
        }

        /**
         * Sets the number of channels each pixels has
         * @param channels
         * @return
         */
        misa_ome_tiff_description_modifier &pixel_channels(std::vector<size_t> channels) {
            change_series(m_series);
            if(m_channels[m_series].empty())
                throw std::runtime_error("Please set the number of channels in the channel axis first!");
            if (channels.size() == 1) {
                for (auto &s : m_channels[m_series]) {
                    s = channels.at(0);
                }
            } else {
                if (channels.size() != m_channels[m_series].size())
                    throw std::runtime_error(
                            "The size of the provided channel configuration should match the number of planes in channel axis! Please set the number of channels in the channel axis first!");
                m_channels[m_series] = std::move(channels);
            }

            update_channels();

            return *this;
        }

        /**
         * Sets the number of channels each pixels has
         * Requires an already set number of planes in channel axis
         * @param channels
         * @return
         */
        misa_ome_tiff_description_modifier &pixel_channels(size_t channels) {
            std::vector<size_t> c = {channels};
            return pixel_channels(std::move(c));
        }

        /**
         * Sets the number of channels and the pixel type
         * @param channels
         * @param t_pixel_type
         * @return
         */
        misa_ome_tiff_description_modifier &of(size_t channels, const ome::xml::model::enums::PixelType &t_pixel_type) {
            pixel_channels(channels);
            return pixel_channel_type(t_pixel_type);
        }

        /**
         * Sets the number of channels and the pixel type from an OpenCV type
         * @param opencv_type
         * @return
         */
        misa_ome_tiff_description_modifier &of_opencv(int opencv_type) {
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
        misa_ome_tiff_description_modifier &of_coixx() {
            pixel_channels(Image::color_type::channels);
            return pixel_channel_type(coixx_to_ome_pixel_type<Image>());
        }

        /**
         * Sets the number of planes in the Z (depth) axis
         * @param size
         * @return
         */
        misa_ome_tiff_description_modifier &depth(size_t size) {
            change_series(m_series);
            m_result.metadata->setPixelsSizeZ(size, m_series);
            return *this;
        }

        /**
         * Sets the number of planes in the channel axis
         * This is not the number of channels each pixel has
         * @param size
         * @return
         */
        misa_ome_tiff_description_modifier &channels(size_t size, size_t num_channels = 1) {
            change_series(m_series);
            if(m_channels[m_series].size() < size) {
                while(m_channels[m_series].size() < size + 1) {
                    m_channels[m_series].push_back(num_channels);
                }
            }
            else if(m_channels[m_series].size() > size) {
                m_channels[m_series].resize(size); // Remove the last
            }
            return *this;
        }

        /**
         * Sets the number of planes in the time axis
         * @param size
         * @return
         */
        misa_ome_tiff_description_modifier &duration(size_t size) {
            change_series(m_series);
            m_result.metadata->setPixelsSizeT(size, m_series);
            return *this;
        }

        /**
         * Sets the width of the planes
         * @param size
         * @return
         */
        misa_ome_tiff_description_modifier &width(size_t size) {
            change_series(m_series);
            m_result.metadata->setPixelsSizeX(size, m_series);
            return *this;
        }

        /**
         * Sets the height of the planes
         * @param size
         * @return
         */
        misa_ome_tiff_description_modifier &height(size_t size) {
            change_series(m_series);
            m_result.metadata->setPixelsSizeY(size, m_series);
            return *this;
        }

        /**
         * Sets the width and height
         * @param w
         * @param h
         * @return
         */
        misa_ome_tiff_description_modifier &of_size(size_t w, size_t h) {
            return width(w).height(h);
        }

        /**
         * Modifies the description with a custom function
         * @tparam Function
         * @param t_function Function that takes a ome::xml::meta::OMEXMLMetadata
         * @return
         */
        template<class Function>
        misa_ome_tiff_description_modifier &modify(const Function &t_function) {
            t_function(*m_result.metadata);
            return *this;
        }

    private:

        size_t m_series = 0;
        std::vector<std::vector<size_t>> m_channels;
        misa_ome_tiff_description m_result;

        /**
         * Writes the channel configuration into the metadata
         */
        void update_channels() {
            for(size_t series = 0; series < m_channels.size(); ++series) {
                const auto &channels = m_channels[series];
                size_t sizeC = std::accumulate(channels.begin(), channels.end(), size_t(0));

                m_result.metadata->setPixelsSizeC(sizeC, series);

                const size_t effSizeC = channels.size();

                for (size_t c = 0; c < effSizeC; ++c)
                {
                    size_t rgbC = channels.at(c);

                    m_result.metadata->setChannelID(ome::files::createID("Channel", series, c), series, c);
                    m_result.metadata->setChannelSamplesPerPixel(static_cast<int>(rgbC), series, c);
                }
            }
        }
    };
}




