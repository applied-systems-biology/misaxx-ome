#include <misaxx_ome/helpers/ome_helpers.h>
#include <ome/files/PixelProperties.h>
#include "ome_helpers.h"


std::shared_ptr<ome::files::CoreMetadata>
misaxx_ome::helpers::create_ome_core_metadata(size_t size_X, size_t size_Y, size_t size_Z, size_t size_T,
                                              std::vector<size_t> size_C, ome::xml::model::enums::PixelType pixel_type) {
    std::shared_ptr<ome::files::CoreMetadata> core(std::make_shared<ome::files::CoreMetadata>());
    core->sizeX = size_X;
    core->sizeY = size_Y;
    core->sizeC = std::move(size_C);
    core->sizeZ = size_Z;
    core->sizeT = size_T;
    core->interleaved = false;
    core->dimensionOrder = ome::xml::model::enums::DimensionOrder::XYZTC;
    core->pixelType = pixel_type;
    core->bitsPerPixel = ome::files::bitsPerPixel(pixel_type);
    return core;
}

std::shared_ptr<ome::xml::meta::OMEXMLMetadata>
misaxx_ome::helpers::create_ome_xml_metadata(size_t size_X, size_t size_Y, size_t size_Z, size_t size_T,
                                             std::vector<size_t> size_C, ome::xml::model::enums::PixelType pixel_type) {
    /* create-metadata-start */
    // OME-XML metadata store.
    auto meta = std::make_shared<ome::xml::meta::OMEXMLMetadata>();

    // Create simple CoreMetadata and use this to set up the OME-XML
    // metadata.  This is purely for convenience in this example; a
    // real writer would typically set up the OME-XML metadata from an
    // existing MetadataRetrieve instance or by hand.
    std::vector<std::shared_ptr<ome::files::CoreMetadata>> seriesList = { create_ome_core_metadata(size_X, size_Y, size_Z, size_T, std::move(size_C), pixel_type) };

    ome::files::fillMetadata(*meta, seriesList);
    /* create-metadata-end */

    return meta;
}

std::shared_ptr<ome::files::CoreMetadata>
misaxx_ome::helpers::create_ome_core_metadata(const ome::xml::meta::OMEXMLMetadata &t_metadata, size_t series) {

    using namespace ome::files;

    auto core = std::make_shared<ome::files::CoreMetadata>();
    
    core->sizeX = t_metadata.getPixelsSizeX(series);
    core->sizeY = t_metadata.getPixelsSizeY(series);
    core->sizeZ = t_metadata.getPixelsSizeZ(series);
    core->sizeT = t_metadata.getPixelsSizeT(series);
    core->pixelType = t_metadata.getPixelsType(series);
    core->interleaved = false;
    core->indexed = false;
    core->bitsPerPixel = bitsPerPixel(core->pixelType);

    for(size_t c = 0; c < t_metadata.getChannelCount(series); ++c) {
        core->sizeC.push_back(t_metadata.getChannelSamplesPerPixel(series, c));
    }

    return core;
}
