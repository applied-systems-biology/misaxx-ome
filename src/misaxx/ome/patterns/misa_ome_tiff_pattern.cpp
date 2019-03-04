#include <misaxx/ome/patterns/misa_ome_tiff_pattern.h>

using namespace misaxx;
using namespace misaxx::ome;

misa_ome_tiff_pattern::misa_ome_tiff_pattern() : misaxx::misa_file_pattern({ ".tif", ".tiff" }) {

}

std::string misa_ome_tiff_pattern::get_documentation_name() const {
    return "OME TIFF pattern";
}

std::string misa_ome_tiff_pattern::get_documentation_description() const {
    return "Finds a *.tiff/*.tif file";
}
