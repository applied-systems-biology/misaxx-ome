//
// Created by rgerst on 22.11.18.
//


#pragma once
#include <ome/files/VariantPixelBuffer.h>
#include <ome/files/FormatWriter.h>
#include <ome/xml/model/primitives/Quantity.h>
#include <ome/xml/model/enums.h>
#include <ome/files/FormatReader.h>
#include <ome/files/out/OMETIFFWriter.h>
#include <ome/files/detail/OMETIFF.h>
#include <ome/files/tiff/TIFF.h>
#include <ome/files/tiff/IFD.h>
#include <ome/files/tiff/Tags.h>
#include <ome/files/tiff/Field.h>
#include <ome/files/MetadataTools.h>
#include <ome/files/CoreMetadata.h>
#include <opencv2/opencv.hpp>

namespace misaxx_ome {

    /**
     * Converts an OpenCV pixel depth to an OME pixel type
     * @param opencv_depth
     * @return
     */
    inline ome::xml::model::enums::PixelType opencv_depth_to_ome_pixel_type(int opencv_depth) {
        switch (opencv_depth) {
            case CV_8U:
                return ome::xml::model::enums::PixelType::UINT8;
            case CV_8S:
                return ome::xml::model::enums::PixelType::INT8;
            case CV_16U:
                return ome::xml::model::enums::PixelType::UINT16;
            case CV_16S:
                return ome::xml::model::enums::PixelType::INT16;
            case CV_32S:
                return ome::xml::model::enums::PixelType::INT32;
            case CV_32F:
                return ome::xml::model::enums::PixelType::FLOAT;
            case CV_64F:
                return ome::xml::model::enums::PixelType::DOUBLE;
            default:
                throw std::runtime_error("Unsupported OpenCV pixel depth!");
        }
    }

    inline ome::files::pixel_size_type opencv_depth_to_ome_pixel_size(int opencv_depth) {
        switch (opencv_depth) {
            case CV_8U:
                return sizeof(uchar) * 8;
            case CV_8S:
                return sizeof(char) * 8;
            case CV_16U:
                return sizeof(ushort) * 8;
            case CV_16S:
                return sizeof(short) * 8;
            case CV_32S:
                return sizeof(int) * 8;
            case CV_32F:
                return sizeof(float) * 8;
            case CV_64F:
                return sizeof(double) * 8;
            default:
                throw std::runtime_error("Unsupported OpenCV pixel depth!");
        }
    }

    inline std::shared_ptr<ome::files::CoreMetadata> opencv_to_ome_series_metadata(const cv::Mat &opencv_image, ome::files::dimension_size_type num_images = 1) {
        std::shared_ptr<ome::files::CoreMetadata> core(std::make_shared<ome::files::CoreMetadata>());
        core->sizeX = opencv_image.cols;
        core->sizeY = opencv_image.rows;
        core->sizeC.clear(); // defaults to 1 channel with 1 subchannel; clear this
        core->sizeC.push_back(opencv_image.channels());
        core->sizeZ = num_images;
        core->sizeT = 1;
        core->interleaved = false;
        core->dimensionOrder = ome::xml::model::enums::DimensionOrder::XYZTC;
        core->pixelType = opencv_depth_to_ome_pixel_type(opencv_image.depth());
        core->bitsPerPixel = opencv_depth_to_ome_pixel_size(opencv_image.depth());
        return core;
    }

    inline std::shared_ptr<ome::xml::meta::OMEXMLMetadata> opencv_to_ome_metadata(const cv::Mat &opencv_image, ome::files::dimension_size_type num_images = 1, ome::files::dimension_size_type num_series = 1)
    {
        /* create-metadata-start */
        // OME-XML metadata store.
        auto meta = std::make_shared<ome::xml::meta::OMEXMLMetadata>();

        // Create simple CoreMetadata and use this to set up the OME-XML
        // metadata.  This is purely for convenience in this example; a
        // real writer would typically set up the OME-XML metadata from an
        // existing MetadataRetrieve instance or by hand.
        std::vector<std::shared_ptr<ome::files::CoreMetadata>> seriesList;

        for(ome::files::dimension_size_type i = 0; i < num_series; ++i) {
            seriesList.push_back(opencv_to_ome_series_metadata(opencv_image, num_images));
        }

        ome::files::fillMetadata(*meta, seriesList);
        /* create-metadata-end */

        return meta;
    }

    template<typename RawType, int OMEPixelType> inline void opencv_to_ome_detail(const cv::Mat &opencv_image, ome::files::FormatWriter &ome_writer, ome::files::dimension_size_type index) {
        using namespace ome::files;
        using namespace ome::xml::model::enums;
        const int size_x = opencv_image.cols;
        const int size_y = opencv_image.rows;
        const int channels = opencv_image.channels();
        auto buffer = std::make_shared<PixelBuffer<typename PixelProperties<OMEPixelType>::std_type>> (boost::extents[size_x][size_y][1][1][1][channels][1][1][1],
                                                               opencv_depth_to_ome_pixel_type(opencv_image.depth()),
                                                               ome::files::ENDIAN_NATIVE,
                                                               PixelBufferBase::make_storage_order(DimensionOrder::XYZTC, false));
        PixelBufferBase::indices_type idx;
        std::fill(idx.begin(), idx.end(), 0);

        for(int y = 0; y < opencv_image.rows; ++y) {
            const auto *ptr = opencv_image.ptr<RawType>(y);
            for(int x = 0; x < opencv_image.cols; ++x) {
                idx[DIM_SPATIAL_X] = x;
                idx[DIM_SPATIAL_Y] = y;
                for(int c = 0; c < channels; ++c) {
                    idx[DIM_SUBCHANNEL] = c;
                    buffer->at(idx) = ptr[x * channels + c];
                }
            }
        }

        VariantPixelBuffer vbuffer(buffer);
        ome_writer.saveBytes(index, vbuffer);
    }

    inline void opencv_to_ome(const cv::Mat &opencv_image, ome::files::FormatWriter &ome_writer, ome::files::dimension_size_type index) {

        using namespace ome::xml::model::enums;

        switch (opencv_image.depth()) {
            case CV_8U: {
                opencv_to_ome_detail<uchar, PixelType::UINT8>(opencv_image, ome_writer, index);
            }
            break;
            case CV_8S: {
                opencv_to_ome_detail<char, PixelType::INT8>(opencv_image, ome_writer, index);
            }
            break;
            case CV_16U: {
                opencv_to_ome_detail<ushort, PixelType::UINT16>(opencv_image, ome_writer, index);
            }
            break;
            case CV_16S: {
                opencv_to_ome_detail<short, PixelType::INT16 >(opencv_image, ome_writer, index);
            }
            break;
            case CV_32S: {
                opencv_to_ome_detail<int, PixelType::INT32>(opencv_image, ome_writer, index);
            }
            break;
            case CV_32F: {
                opencv_to_ome_detail<float, PixelType::FLOAT>(opencv_image, ome_writer, index);
            }
            break;
            case CV_64F: {
                opencv_to_ome_detail<double, PixelType::DOUBLE >(opencv_image, ome_writer, index);
            }
            break;
            default:
                throw std::runtime_error("Unsupported OpenCV pixel depth!");
        }
    }
}