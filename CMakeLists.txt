cmake_minimum_required(VERSION 3.11)
project(misaxx-ome VERSION 0.1.0 DESCRIPTION "MISA++ OME Integration")

find_package(misaxx-core REQUIRED)
find_package(misaxx-imaging REQUIRED)
find_package(Boost 1.63 COMPONENTS REQUIRED log_setup log iostreams)
find_package(Threads REQUIRED)
find_package(OMEFiles 0.5.0 REQUIRED)
find_package(OMEXML 5.6.0 REQUIRED)

add_library(misaxx-ome
        include/misaxx-ome/misa_ome.h
        src/misaxx-ome/io/ome_to_opencv.h
        src/misaxx-ome/io/ome_to_opencv.cpp
        include/misaxx-ome/caches/misa_ome_tiff_cache.h
        src/misaxx-ome/caches/misa_ome_tiff_cache.cpp
        include/misaxx-ome/caches/misa_ome_plane_cache.h
        src/misaxx-ome/caches/misa_ome_plane_cache.cpp
        include/misaxx-ome/patterns/misa_ome_tiff_pattern.h
        src/misaxx-ome/patterns/misa_ome_tiff_pattern.cpp
        include/misaxx-ome/descriptions/misa_ome_tiff_description.h
        src/misaxx-ome/descriptions/misa_ome_tiff_description.cpp
        src/misaxx-ome/io/opencv_to_ome.h
        src/misaxx-ome/io/opencv_to_ome.cpp
        include/misaxx-ome/io/ome_tiff_io.h
        src/misaxx-ome/io/ome_tiff_io.cpp
        include/misaxx-ome/io/json_ome_pixel_type.h
        src/misaxx-ome/io/json_ome_pixel_type.cpp
        include/misaxx-ome/ome_helpers.h
        src/misaxx-ome/ome_helpers.cpp
        include/misaxx-ome/descriptions/misa_ome_plane_description.h
        src/misaxx-ome/descriptions/misa_ome_plane_description.cpp
        src/misaxx-ome/accessors/misa_ome_plane.cpp
        include/misaxx-ome/accessors/misa_ome_plane.h
        src/misaxx-ome/accessors/misa_ome_tiff.cpp
        include/misaxx-ome/accessors/misa_ome_tiff.h
        src/misaxx-ome/io/ome_to_ome.h
        src/misaxx-ome/io/ome_to_ome.cpp
        include/misaxx-ome/misa_ome_tiff_description_builder.h
        src/misaxx-ome/misa_ome_tiff_description_builder.cpp
        include/misaxx-ome/io/coixx_to_ome.h
        src/misaxx-ome/io/coixx_to_ome.cpp
        src/misaxx-ome/attachments/misa_ome_planes_location.cpp
        include/misaxx-ome/attachments/misa_ome_planes_location.h
        src/misaxx-ome/io/json_ome_quantity.cpp
        include/misaxx-ome/io/json_ome_quantity.h
        src/misaxx-ome/misa_ome_tiff_description_modifier.cpp
        include/misaxx-ome/misa_ome_tiff_description_modifier.h
        src/misaxx-ome/attachments/misa_ome_unit.cpp
        include/misaxx-ome/attachments/misa_ome_unit.h
        include/misaxx-ome/attachments/misa_ome_voxel_size.h
        src/misaxx-ome/attachments/misa_ome_voxel_size.cpp
        src/misaxx-ome/attachments/misa_ome_pixel_count.cpp
        include/misaxx-ome/attachments/misa_ome_pixel_count.h
        src/misaxx-ome/attachments/misa_ome_voxel.cpp
        include/misaxx-ome/attachments/misa_ome_voxel.h)

misaxx_make_shared(misaxx misaxx-ome)
misaxx_add_compiler_warnings(misaxx-ome)

target_link_libraries(misaxx-ome PUBLIC OME::Files misaxx::misaxx-core misaxx::misaxx-imaging)
