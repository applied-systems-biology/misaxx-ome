//
// Created by rgerst on 14.11.18.
//


#pragma once

#include <ome/files/VariantPixelBuffer.h>
#include <ome/files/FormatReader.h>
#include <opencv2/opencv.hpp>
#include <misaxx-ome/descriptions/misa_ome_plane_description.h>

namespace misaxx_ome {

    /**
     * Converts a OME variant pixel buffer into a cv::Mat
     * @tparam RawType
     * @param ome_buffer
     * @param size_x
     * @param size_y
     * @param channels
     * @param opencv_type
     * @return
     */
    template<typename RawType> inline cv::Mat ome_to_opencv_detail(const ome::files::VariantPixelBuffer &ome_buffer, int size_x, int size_y, int channels, int opencv_type) {
        using namespace ome::files;
        cv::Mat result(size_y, size_x, opencv_type, cv::Scalar::all(0));

        if(!result.isContinuous())
            throw std::runtime_error("cv::Mat must be continuous!");

        const auto &src_array = ome_buffer.array<RawType>();

        PixelBufferBase::indices_type idx;
        std::fill(idx.begin(), idx.end(), 0);

        for(int y = 0; y < result.rows; ++y) {
            auto *ptr = result.ptr<RawType>(y);
            for(int x = 0; x < result.cols; ++x) {
                idx[DIM_SPATIAL_X] = x;
                idx[DIM_SPATIAL_Y] = y;
                for(int c = 0; c < channels; ++c) {
                    idx[DIM_SUBCHANNEL] = c;
                    ptr[x * channels + c] = src_array(idx);
                }
            }
        }
        return result;
    }
    
    /**
     * Converts a OME variant pixel buffer into a cv::Mat
     * @param ome
     * @return
     */
    extern cv::Mat ome_to_opencv(const ome::files::FormatReader &ome_reader, const misa_ome_plane_description &index);

}