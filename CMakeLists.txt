cmake_minimum_required(VERSION 3.11)
include(misaxx_cmake/misaxx.cmake)

project(misaxx_ome)

set(Boost_USE_STATIC_LIBS ON)

find_package(Boost 1.63 COMPONENTS REQUIRED log_setup log iostreams)
find_package(Threads REQUIRED)
find_package(OMEFiles 0.5.0 REQUIRED)
find_package(OMEXML 5.6.0 REQUIRED)

add_library(misaxx_ome
        misaxx_ome/misa_ome.h
        misaxx_ome/misa_ome.cpp
        misaxx_ome/io/ome_to_opencv.h
        misaxx_ome/io/ome_to_opencv.cpp
        misaxx_ome/caches/misa_ome_tiff_cache.h
        misaxx_ome/caches/misa_ome_tiff_cache.cpp
        misaxx_ome/caches/misa_ome_plane_cache.h
        misaxx_ome/caches/misa_ome_plane_cache.cpp
        misaxx_ome/patterns/misa_ome_tiff_pattern.h
        misaxx_ome/patterns/misa_ome_tiff_pattern.cpp
        misaxx_ome/descriptions/misa_ome_tiff_description.h
        misaxx_ome/descriptions/misa_ome_tiff_description.cpp
        misaxx_ome/io/opencv_to_ome.h
        misaxx_ome/io/opencv_to_ome.cpp
        misaxx_ome/io/ome_tiff_io.h
        misaxx_ome/io/ome_tiff_io.cpp
        misaxx_ome/descriptions/misa_ome_series_description.h
        misaxx_ome/descriptions/misa_ome_series_description.cpp
        misaxx_ome/io/json_ome_pixel_type.h
        misaxx_ome/io/json_ome_pixel_type.cpp
        misaxx_ome/helpers/ome_helpers.h
        misaxx_ome/helpers/ome_helpers.cpp
        misaxx_ome/descriptions/misa_ome_plane_description.h
        misaxx_ome/descriptions/misa_ome_plane_description.cpp
        misaxx_ome/accessors/misa_ome_plane.cpp
        misaxx_ome/accessors/misa_ome_plane.h
        misaxx_ome/accessors/misa_ome_tiff.cpp
        misaxx_ome/accessors/misa_ome_tiff.h
        misaxx_ome/io/ome_to_ome.h
        misaxx_ome/io/ome_to_ome.cpp
        misaxx_ome/helpers/misa_ome_tiff_description_builder.h
        misaxx_ome/helpers/misa_ome_tiff_description_builder.cpp
        misaxx_ome/io/coixx_to_ome.h
        misaxx_ome/io/coixx_to_ome.cpp misaxx_ome/attachments/misa_ome_quantity.cpp misaxx_ome/attachments/misa_ome_quantity.h misaxx_ome/attachments/misa_ome_location.cpp misaxx_ome/attachments/misa_ome_location.h misaxx_ome/attachments/misa_ome_pixels.cpp misaxx_ome/attachments/misa_ome_pixels.h misaxx_ome/attachments/misa_ome_voxel.cpp misaxx_ome/attachments/misa_ome_voxel.h misaxx_ome/io/json_ome_quantity.cpp misaxx_ome/io/json_ome_quantity.h misaxx_ome/helpers/misa_ome_tiff_description_modifier.cpp misaxx_ome/helpers/misa_ome_tiff_description_modifier.h)

target_link_libraries(misaxx_ome PUBLIC OME::Files)
misa_module(misaxx_ome)
