#include "ome_to_opencv.h"

using namespace misaxx_ome;

cv::Mat misaxx_ome::ome_to_opencv(const ome::files::FormatReader &ome_reader, const misa_ome_plane_description &index) {

    using namespace ome::xml::model::enums;

    int size_x = static_cast<int>(ome_reader.getSizeX());
    int size_y = static_cast<int>(ome_reader.getSizeY());
    int channels = static_cast<int>(ome_reader.getRGBChannelCount(index.c));

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
