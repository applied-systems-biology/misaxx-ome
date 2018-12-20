#include <misaxx-ome/descriptions/misa_ome_tiff_description.h>
#include <misaxx/runtime/misa_runtime_base.h>

using namespace misaxx;
using namespace misaxx_ome;

void misa_ome_tiff_description::from_json(const nlohmann::json &t_json) {
    misa_file_description::from_json(t_json);
    metadata = ome::files::createOMEXMLMetadata(t_json["ome-xml-metadata"].get<std::string>());
}

void misa_ome_tiff_description::to_json(nlohmann::json &t_json) const {
    misa_file_description::to_json(t_json);
    if(!misaxx::misa_runtime_base::instance().is_simulating()) {
        t_json["ome-xml-metadata"] = metadata->dumpXML();
    }
}

void misa_ome_tiff_description::to_json_schema(const misaxx::misa_json_schema &t_schema) const {
    misa_file_description::to_json_schema(t_schema);
    t_schema.resolve("ome-xml-metadata").declare_optional<std::string>();
}

void
misa_ome_tiff_description::build_serialization_id_hierarchy(std::vector<misaxx::misa_serialization_id> &result) const {
    misa_file_description::build_serialization_id_hierarchy(result);
    result.emplace_back(misaxx::misa_serialization_id("misa_ome", "descriptions/ome-tiff"));
}
