////
//// Created by rgerst on 30.07.18.
////
//
//#include <misaxx/runtime/misa_cli.h>
//#include <ome/xml/model/primitives/Quantity.h>
//#include <ome/xml/model/enums.h>
//#include <ome/files/FormatReader.h>
//#include <ome/files/in/OMETIFFReader.h>
//#include <ome/files/detail/OMETIFF.h>
//#include <ome/files/tiff/TIFF.h>
//#include <ome/files/tiff/IFD.h>
//#include <ome/files/tiff/Tags.h>
//#include <ome/files/tiff/Field.h>
//#include <ome/files/MetadataTools.h>
//#include <ome/files/CoreMetadata.h>
//#include <misaxx_ome/io/ome_to_opencv.h>
//#include <misaxx_ome/io/opencv_to_ome.h>
//#include <coixx/toolbox/toolbox_utils.h>
//#include <misaxx_ome/caches/misa_ome_tiff.h>
//#include <misaxx/misa_cached_data.h>
//#include <coixx/image.h>
//#include <coixx/toolbox/toolbox_io.h>
#include <iostream>
#include <misaxx_ome/io/ome_read_write_tiff.h>
//
//using namespace misaxx;
//using namespace misaxx_ome;
//using namespace ome::files;
//using namespace ome::files::tiff;
//
//struct misa_ome_test_declaration : public misa_module_declaration {
//
//    misa_cached_data<misa_ome_tiff> tiff;
//    misa_cached_data<misa_ome_tiff> out_tiff;
//
//    void init_data() override {
//        tiff.suggest_import_location(filesystem, "/");
//        out_tiff.suggest_export_location(filesystem, "out_tiff", tiff.describe());
//    }
//};
//
//struct misa_ome_test : public misa_module<misa_ome_test_declaration> {
//    using misa_module<misa_ome_test_declaration>::misa_module;
//
//    void misa_init() override {
//        for(size_t i = 0; i < misa_ome_tiff::get_num_images(tiff, 0); ++i) {
//            std::cout << i << std::endl;
//            cv::Mat img = misa_ome_tiff::read_plane(tiff, 0, i);
//            misa_ome_tiff::write_plane(out_tiff, std::move(img), 0, i);
//        }
//    }
//};
//
//int main(int argc, const char** argv) {
//    misa_cli<misa_multiobject_root<misa_ome_test>> cli("test");
//    return cli.prepare_and_run(argc, argv);
//}

int main(int argc, const char** argv) {

    using namespace misaxx_ome;
    ome_read_write_tiff tiff("/home/rgerst/tmp/ome/input/object0/example_data.ome.tif");
    std::cout << tiff.get_metadata()->getImageCount() << std::endl;
    auto reader_access = tiff.get_reader();

    for(size_t i = 0; i < tiff.get_metadata()->getImageCount(); ++i) {
        reader_access.value->setSeries(i);
        auto index = misa_ome_plane_location(i);
        auto end = misa_ome_plane_location::get_end(*reader_access.value, i);
        for(; index != end; index.increment_in_series(end)) {
            std::cout << index.t << " " << index.c << " " << index.z << std::endl;
        }
    }

    return 0;
}
