//
// Created by rgerst on 14.11.18.
//


#pragma once

#include <ome/files/VariantPixelBuffer.h>
#include <ome/files/FormatReader.h>
#include <opencv2/opencv.hpp>
#include <misaxx_ome/descriptions/misa_ome_plane_location.h>

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
    inline cv::Mat ome_to_opencv(const ome::files::FormatReader &ome_reader, const misa_ome_plane_location &index) {
        
        using namespace ome::xml::model::enums;
        
        int size_x = static_cast<int>(ome_reader.getSizeX());
        int size_y = static_cast<int>(ome_reader.getSizeY());
        int channels = static_cast<int>(ome_reader.getSizeC());

        ome::files::VariantPixelBuffer ome_buffer;
        ome_reader.openBytes(index.index_within(ome_reader), ome_buffer);

        switch(ome_buffer.pixelType()) {
            case PixelType::UINT8: 
                return ome_to_opencv_detail<uchar>(ome_buffer, size_x, size_y, channels, CV_8UC(channels));
            case PixelType::INT8:
                return ome_to_opencv_detail<char>(ome_buffer, size_x, size_y, channels, CV_8SC(channels));
            case PixelType::UINT16:
                return ome_to_opencv_detail<ushort>(ome_buffer, size_x, size_y, channels, CV_16UC(channels));
            case PixelType::INT16:
                return ome_to_opencv_detail<short>(ome_buffer, size_x, size_y, channels, CV_16SC(channels));
            case PixelType::INT32:
                return ome_to_opencv_detail<int>(ome_buffer, size_x, size_y, channels, CV_32SC(channels));
            case PixelType::FLOAT:
                return ome_to_opencv_detail<float>(ome_buffer, size_x, size_y, channels, CV_32FC(channels));
            case PixelType::DOUBLE:
                return ome_to_opencv_detail<double>(ome_buffer, size_x, size_y, channels, CV_64FC(channels));
            default:
                throw std::runtime_error("OpenCV does not support this pixel type!");
        }
    }

}