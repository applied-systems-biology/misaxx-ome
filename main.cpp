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
#include <misaxx_ome/caches/misa_ome_tiff_cache.h>
#include <misaxx/misa_cached_data.h>
#include <coixx/image.h>
#include <coixx/toolbox/toolbox_io.h>
#include <iostream>
#include <misaxx_ome/io/ome_tiff_io.h>
#include <misaxx_ome/accessors/misa_ome_tiff.h>
#include <coixx/toolbox/toolbox_channels.h>

using namespace misaxx;
using namespace misaxx_ome;
using namespace ome::files;
using namespace ome::files::tiff;

struct misa_ome_test_declaration : public misa_module_declaration {
    misa_ome_tiff<> tiff;
    misa_ome_tiff<> out_tiff;

    void init_data() override {
        tiff.suggest_import_location(filesystem, "/");
        out_tiff.suggest_export_location(filesystem, "out_tiff", tiff.derive().of_coixx<coixx::images::bgr8u>());
    }
};

struct misa_ome_test : public misa_module<misa_ome_test_declaration> {
    using misa_module<misa_ome_test_declaration>::misa_module;

    void misa_init() override {
        for(size_t i = 0; i < tiff.size(); ++i) {
            std::cout << tiff.at(i).get_data_description() << std::endl;

            coixx::images::grayscale8u input(tiff.at(i).clone());
            coixx::images::grayscale8u black(input.get_size(), coixx::colors::grayscale8u::black());

            auto rgb = coixx::toolbox::channels::merge<coixx::images::bgr8u>(input, black, black);
            out_tiff.at(i).write(rgb.get_image());

//            auto read_access = tiff.at(i).access_readonly();
//            auto write_access = out_tiff.at(i).access_write();
//
//            write_access.set(read_access.get());
        }
    }
};

int main(int argc, const char** argv) {
    misa_cli<misa_multiobject_root<misa_ome_test>> cli("test");
    return cli.prepare_and_run(argc, argv);
}

//int main(int argc, const char** argv) {
//
//    using namespace misaxx_ome;
////    ome_tiff_io tiff("/home/rgerst/tmp/ome/test/object0/example_data_Z0_C0_T0.ome.tif");
//    ome_tiff_io tiff("/home/rgerst/tmp/ome/1/123_stack.ome.tif");
//    ome_tiff_io tiff_write("/home/rgerst/tmp/ome/test/123/123_stack.ome.tif", tiff.get_metadata());
//
//    for(size_t p = 0; p < tiff.get_num_planes(0); ++p) {
////        auto i = tiff.get_num_planes(0) - p - 1;
//        auto i = p;
//        const auto loc = misa_ome_plane_description(0, 0, 0, 0);
//        cv::Mat plane = tiff.read_plane(loc);
//        std::cout << "Write plane " << p << std::endl;
//        tiff_write.write_plane(plane, misa_ome_plane_description(0, p, 0, 0));
//    }
//
//    tiff_write.close();
//
////    ome_tiff_io tiff("/home/rgerst/tmp/ome/test/split_test/ome_example_small_S0_Z0_C0_T0.ome.tif");
////    cv::Mat plane = tiff.read_plane(misa_ome_plane_location(0, 0, 0, 0));
////    tiff.write_plane(plane, misa_ome_plane_location(0, 0, 0, 0));
////
////    tiff.read_plane(misa_ome_plane_location(0, 0, 0, 0));
//
//    return 0;
//}
