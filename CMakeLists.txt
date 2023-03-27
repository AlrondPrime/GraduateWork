include(Properties.cmake)

cmake_minimum_required(VERSION 3.24)
project(GraduateWork)

set(CMAKE_CXX_STANDARD 14)

set(Boost_NO_WARN_NEW_VERSIONS 1)
#set(Boost_DEBUG ON)
#set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.81.0 COMPONENTS filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(PCH src/pch.h)
set(SOURCE_FILES
        src/main.cpp
        "src/MaskMultiply.hpp"
        "src/Byte.hpp"
        "src/Compression alternatives/Byte_RLE.hpp"
        "src/Compression alternatives/Bit_RLE.hpp"
        src/TestMask.hpp
        src/CLI.hpp)

add_executable(GraduateWork ${SOURCE_FILES})
target_link_libraries(GraduateWork ${Boost_LIBRARIES})
target_precompile_headers(GraduateWork
        PRIVATE ${PCH})