//
// Created by rgerst on 28.11.18.
//

#include <misaxx/ome/accessors/misa_ome_plane.h>
#include "../utils/ome_tiff_io.h"

cv::Mat misaxx::ome::misa_ome_plane::clone() const {
    return this->access_readonly().get().clone();
}

void misaxx::ome::misa_ome_plane::write(cv::Mat t_data) {
    this->access_write().set(std::move(t_data));
}

const misaxx::ome::misa_ome_plane_description &misaxx::ome::misa_ome_plane::get_plane_location() const {
    return this->data->get_plane_location();
}

size_t misaxx::ome::misa_ome_plane::get_size_x() const {
    return this->data->get_tiff_io()->get_size_x(get_plane_location().series);
}

size_t misaxx::ome::misa_ome_plane::get_size_y() const {
    return this->data->get_tiff_io()->get_size_y(get_plane_location().series);
}

std::shared_ptr<::ome::xml::meta::OMEXMLMetadata> misaxx::ome::misa_ome_plane::get_ome_metadata() const {
    return this->data->get_tiff_io()->get_metadata();
}
