//
// Created by rgerst on 05.12.18.
//

#include <misaxx-ome/attachments/misa_ome_planes_location.h>

using namespace misaxx;
using namespace misaxx_ome;

misa_ome_planes_location::misa_ome_planes_location(misa_cached_data_base &t_cache,
                                                 std::vector<misa_ome_plane_description> t_planes) : misa_location(t_cache),
                                                 planes(std::move(t_planes)){

}

void misa_ome_planes_location::from_json(const nlohmann::json &t_json) {
    misa_location::from_json(t_json);
    planes = t_json["ome-planes"].get<std::vector<misa_ome_plane_description>>();
}

void misa_ome_planes_location::to_json(nlohmann::json &t_json) const {
    misa_location::to_json(t_json);
    t_json["ome-planes"] = planes;
}

void misa_ome_planes_location::to_json_schema(const misaxx::misa_json_schema &t_schema) const {
    misa_location::to_json_schema(t_schema);
    t_schema.resolve("ome-planes").declare_required<std::vector<misa_ome_plane_description>>();
}

void misa_ome_planes_location::build_serialization_id_hierarchy(std::vector<misaxx::misa_serialization_id> &result) const {
    misa_location::build_serialization_id_hierarchy(result);
    result.emplace_back(misaxx::misa_serialization_id("misa_ome", "attachments/planes-location"));
}
