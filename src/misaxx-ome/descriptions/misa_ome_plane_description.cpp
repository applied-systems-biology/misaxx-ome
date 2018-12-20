#include <misaxx-ome/descriptions/misa_ome_plane_description.h>

using namespace misaxx;
using namespace misaxx_ome;

misa_ome_plane_description::misa_ome_plane_description(ome::files::dimension_size_type t_series) :
        series(t_series), z(0), c(0), t(0) {

}

misa_ome_plane_description::misa_ome_plane_description(ome::files::dimension_size_type t_series,
                                                       ome::files::dimension_size_type t_z,
                                                       ome::files::dimension_size_type t_c,
                                                       ome::files::dimension_size_type t_t) :
        series(t_series), z(t_z), c(t_c), t(t_t) {

}

void misa_ome_plane_description::build_serialization_id_hierarchy(std::vector<misa_serialization_id> &result) const {
    misa_data_description::build_serialization_id_hierarchy(result);
    result.emplace_back(misaxx::misa_serialization_id("misa_ome", "descriptions/plane"));
}

std::array<ome::files::dimension_size_type, 3> misa_ome_plane_description::as_zct() const {
    return { z, c, t };
}

ome::files::dimension_size_type misa_ome_plane_description::index_within(const ome::files::FormatReader &reader) const {
    if(reader.getSeries() != series)
        throw std::runtime_error("The reader must be located at the same series as the location description!");
    return reader.getIndex(z, c, t);
}

ome::files::dimension_size_type
misa_ome_plane_description::index_within(const ome::files::out::OMETIFFWriter &writer) const {
    if(writer.getSeries() != series)
        throw std::runtime_error("The reader must be located at the same series as the location description!");
    return writer.getIndex(z, c, t);
}

void misa_ome_plane_description::from_json(const nlohmann::json &t_json) {
    series = t_json["series"];
    z = t_json["z"];
    c = t_json["c"];
    t = t_json["t"];
}

void misa_ome_plane_description::to_json(nlohmann::json &t_json) const {
    t_json["series"] = series;
    t_json["z"] = z;
    t_json["c"] = c;
    t_json["t"] = t;
}

void misa_ome_plane_description::to_json_schema(const misaxx::misa_json_schema &t_schema) const {
    t_schema.resolve("series").declare_required<ome::files::dimension_size_type>();
    t_schema.resolve("z").declare_required<ome::files::dimension_size_type>();
    t_schema.resolve("c").declare_required<ome::files::dimension_size_type>();
    t_schema.resolve("t").declare_required<ome::files::dimension_size_type>();
}

bool misa_ome_plane_description::operator==(const misa_ome_plane_description &rhs) const {
    return series == rhs.series &&
           z == rhs.z &&
           c == rhs.c &&
           t == rhs.t;
}

bool misa_ome_plane_description::operator<(const misa_ome_plane_description &rhs) const {
    return series < rhs.series || z < rhs.z || c < rhs.c || t < rhs.t;
}
