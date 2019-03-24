//
// Created by rgerst on 14.11.18.
//


#pragma once

#include <ome/files/FormatReader.h>
#include <opencv2/opencv.hpp>
#include <misaxx/ome/descriptions/misa_ome_plane_description.h>

namespace misaxx::ome {
    
    /**
     * Converts a OME variant pixel buffer into a cv::Mat
     * @param ome
     * @return
     */
    extern cv::Mat ome_to_opencv(const ::ome::files::FormatReader &ome_reader, const misa_ome_plane_description &index);

}