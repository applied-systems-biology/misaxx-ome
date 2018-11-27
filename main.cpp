//
// Created by rgerst on 30.07.18.
//

#include <misaxx/runtime/misa_cli.h>
#include <ome/xml/model/primitives/Quantity.h>
#include <ome/xml/model/enums.h>
#include <ome/files/FormatReader.h>
#include <ome/files/in/OMETIFFReader.h>
#include <ome/files/detail/OMETIFF.h>
#include <ome/files/tiff/TIFF.h>
#include <ome/files/tiff/IFD.h>
#include <ome/files/tiff/Tags.h>
#include <ome/files/tiff/Field.h>
#include <ome/files/MetadataTools.h>
#include <ome/files/CoreMetadata.h>
#include <misaxx_ome/io/ome_to_opencv.h>
#include <misaxx_ome/io/opencv_to_ome.h>
#include <coixx/toolbox/toolbox_utils.h>
#include <misaxx_ome/caches/misa_ome_tiff.h>
#include <misaxx/misa_cached_data.h>
#include <coixx/image.h>
#include <coixx/toolbox/toolbox_io.h>
#include <misaxx_ome/io/ome_read_write_tiff.h>

using namespace misaxx;
using namespace misaxx_ome;
using namespace ome::files;
using namespace ome::files::tiff;

struct misa_ome_test_declaration : public misa_module_declaration {

    misa_cached_data<misa_ome_tiff> tiff;

    void init_data() override {
        tiff.suggest_import_location(filesystem, "/");
    }
};

struct misa_ome_test : public misa_module<misa_ome_test_declaration> {
    using misa_module<misa_ome_test_declaration>::misa_module;

    void misa_init() override {
        auto access = tiff.access_readonly().get()[0][0].access_readonly();
        coixx::images::grayscale8u img(access.get());
        img << coixx::toolbox::show_and_wait("img");
    }
};

int main(int argc, const char** argv) {
    misa_cli<misa_multiobject_root<misa_ome_test>> cli("test");
    return cli.prepare_and_run(argc, argv);
}
