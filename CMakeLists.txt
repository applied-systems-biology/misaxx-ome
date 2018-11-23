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
        misaxx_ome/misa_ome.cpp misaxx_ome/io/ome_to_opencv.h misaxx_ome/caches/misa_ome_readonly_tiff.h misaxx_ome/caches/misa_ome_readonly_image.h misaxx_ome/patterns/misa_ome_tiff_pattern.h misaxx_ome/descriptions/misa_ome_tiff_description.h misaxx_ome/patterns/misa_ome_image_pattern.h misaxx_ome/descriptions/misa_ome_image_description.h misaxx_ome/io/opencv_to_ome.h misaxx_ome/io/ome_read_write_tiff.h misaxx_ome/io/ome_read_write_tiff.cpp)

target_link_libraries(misaxx_ome PUBLIC OME::Files)
misa_module(misaxx_ome)
