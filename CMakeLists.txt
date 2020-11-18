cmake_minimum_required(VERSION 3.17)
project(rm_dupes)

set(CMAKE_CXX_STANDARD 20)

include_directories(oasis)

find_package(Boost 1.71 COMPONENTS program_options regex REQUIRED)
if (Boost_INCLUDE_DIR)
    include_directories(${Boost_INCLUDE_DIR})
else()
    message(FATAL_ERROR "Boost Program Options library not found!")
endif()

add_executable(rm_dupes main.cpp)
target_link_libraries(rm_dupes ${Boost_LIBRARIES})

target_compile_definitions(rm_dupes PRIVATE VERSION=0.90)

find_path(Botan_INCLUDE_DIR botan/botan.h  PATH_SUFFIXES botan-2)
find_library(Botan_LIBRARY NAMES botan botan-2)
if (Botan_INCLUDE_DIR AND Botan_LIBRARY)
    target_include_directories(rm_dupes PUBLIC ${Botan_INCLUDE_DIR})
    target_link_libraries(rm_dupes ${Botan_LIBRARY})
else()
    message(FATAL_ERROR "libbotan not found!")
endif()

find_library(TBB_LIBRARY NAMES tbb)
if (TBB_LIBRARY)
    target_link_libraries(rm_dupes ${TBB_LIBRARY})
else()
    message(FATAL_ERROR "libtbb not found!")
endif()
