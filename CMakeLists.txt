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
        misaxx_ome/misa_ome.cpp)

target_link_libraries(misaxx_ome PUBLIC OME::Files)
misa_module(misaxx_ome)
