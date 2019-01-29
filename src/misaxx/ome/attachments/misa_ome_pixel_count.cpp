//
// Created by rgerst on 13.12.18.
//

#include <misaxx/ome/attachments/misa_ome_pixel_count.h>

using namespace misaxx::ome;
using namespace misaxx;

misa_ome_pixel_count::misa_ome_pixel_count(long t_count) : count(t_count) {
}

void misa_ome_pixel_count::from_json(const nlohmann::json &t_json) {
    count = t_json["count"];
}

void misa_ome_pixel_count::to_json(nlohmann::json &t_json) const {
    misa_locatable::to_json(t_json);
    t_json["count"] = count;
}

void misa_ome_pixel_count::to_json_schema(const misaxx::misa_json_schema &t_schema) const {
    misa_locatable::to_json_schema(t_schema);
    t_schema.resolve("count").declare_required<long>();
}

void misa_ome_pixel_count::build_serialization_id_hierarchy(std::vector<misa_serialization_id> &result) const {
    misa_locatable::build_serialization_id_hierarchy(result);
    result.emplace_back(misa_serialization_id("misa_ome", "attachments/pixel-count"));
}

misaxx::misa_quantity<double, misa_ome_unit_length<3>>
misa_ome_pixel_count::get_volume(const misa_ome_voxel_size &voxel_size) const {
    return voxel_size.get_volume() * static_cast<double>(count);
}
