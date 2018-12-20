//
// Created by rgerst on 07.12.18.
//

#include <misaxx-ome/misa_ome_tiff_description_modifier.h>
#include <ome/files/PixelProperties.h>
#include <misaxx/runtime/misa_runtime_base.h>
#include "io/ome_to_opencv.h"
#include "io/opencv_to_ome.h"

using namespace misaxx;
using namespace misaxx_ome;

misa_ome_tiff_description_modifier::misa_ome_tiff_description_modifier(misa_ome_tiff_description src) : m_result(std::move(src)) {
    // Copy the metadata to allow derivation
    if(!misaxx::misa_runtime_base::instance().is_simulating())
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

misa_ome_tiff_description_modifier::operator misa_ome_tiff_description() {
    return m_result;
}

misa_ome_tiff_description_modifier::operator std::shared_ptr<misaxx::misa_description_storage>() {
    return misaxx::misa_description_storage::with(static_cast<misa_ome_tiff_description>(*this));
}

misa_ome_tiff_description_modifier &misa_ome_tiff_description_modifier::change_series(size_t series) {
    if(misaxx::misa_runtime_base::instance().is_simulating())
        return *this;

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

misa_ome_tiff_description_modifier &
misa_ome_tiff_description_modifier::pixel_channel_type(const ome::xml::model::enums::PixelType &t_pixel_type) {
    if(misaxx::misa_runtime_base::instance().is_simulating())
        return *this;

    change_series(m_series);
    m_result.metadata->setPixelsType(t_pixel_type, m_series);
    m_result.metadata->setPixelsSignificantBits(ome::files::bitsPerPixel(t_pixel_type), m_series);
    return *this;
}

misa_ome_tiff_description_modifier &misa_ome_tiff_description_modifier::pixel_channels(std::vector<size_t> channels) {
    if(misaxx::misa_runtime_base::instance().is_simulating())
        return *this;

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

misa_ome_tiff_description_modifier &misa_ome_tiff_description_modifier::pixel_channels(size_t channels) {
    if(misaxx::misa_runtime_base::instance().is_simulating())
        return *this;

    std::vector<size_t> c = {channels};
    return pixel_channels(std::move(c));
}

misa_ome_tiff_description_modifier &
misa_ome_tiff_description_modifier::of(size_t channels, const ome::xml::model::enums::PixelType &t_pixel_type) {
    if(misaxx::misa_runtime_base::instance().is_simulating())
        return *this;

    pixel_channels(channels);
    return pixel_channel_type(t_pixel_type);
}

misa_ome_tiff_description_modifier &misa_ome_tiff_description_modifier::of_opencv(int opencv_type) {
    if(misaxx::misa_runtime_base::instance().is_simulating())
        return *this;

    cv::Mat m(1, 1, opencv_type);
    pixel_channels(m.channels());
    return pixel_channel_type(opencv_depth_to_ome_pixel_type(m.depth()));
}

misa_ome_tiff_description_modifier &misa_ome_tiff_description_modifier::depth(size_t size) {
    if(misaxx::misa_runtime_base::instance().is_simulating())
        return *this;

    change_series(m_series);
    m_result.metadata->setPixelsSizeZ(size, m_series);
    return *this;
}

misa_ome_tiff_description_modifier &misa_ome_tiff_description_modifier::channels(size_t size, size_t num_channels) {
    if(misaxx::misa_runtime_base::instance().is_simulating())
        return *this;

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

misa_ome_tiff_description_modifier &misa_ome_tiff_description_modifier::duration(size_t size) {
    if(misaxx::misa_runtime_base::instance().is_simulating())
        return *this;

    change_series(m_series);
    m_result.metadata->setPixelsSizeT(size, m_series);
    return *this;
}

misa_ome_tiff_description_modifier &misa_ome_tiff_description_modifier::width(size_t size) {
    if(misaxx::misa_runtime_base::instance().is_simulating())
        return *this;

    change_series(m_series);
    m_result.metadata->setPixelsSizeX(size, m_series);
    return *this;
}

misa_ome_tiff_description_modifier &misa_ome_tiff_description_modifier::height(size_t size) {
    if(misaxx::misa_runtime_base::instance().is_simulating())
        return *this;

    change_series(m_series);
    m_result.metadata->setPixelsSizeY(size, m_series);
    return *this;
}

misa_ome_tiff_description_modifier &misa_ome_tiff_description_modifier::of_size(size_t w, size_t h) {
    if(misaxx::misa_runtime_base::instance().is_simulating())
        return *this;

    return width(w).height(h);
}
