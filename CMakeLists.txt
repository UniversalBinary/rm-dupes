cmake_minimum_required(VERSION 3.16)
project(rm_dupes)

set(CMAKE_CXX_STANDARD 20)

include_directories(oasis)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.75 COMPONENTS program_options regex filesystem thread chrono REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories("C:\\Program Files\\Boost\\stage\\lib")

add_executable(rm_dupes main.cpp "oasis/win32_error.hpp")
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

find_package(fmt CONFIG REQUIRED)
target_link_libraries(rm_dupes fmt::fmt)

if (NOT WIN32)
    find_library(TBB_LIBRARY NAMES tbb)
    if (TBB_LIBRARY)
        target_link_libraries(rm_dupes ${TBB_LIBRARY})
    else()
        message(FATAL_ERROR "libtbb not found!")
    endif()

    target_link_libraries(rm_dupes pthread)
else()
    target_compile_definitions(rm_dupes PRIVATE UNICODE=1)
    target_compile_definitions(rm_dupes PRIVATE _UNICODE=1)
    target_compile_definitions(rm_dupes PRIVATE WIN32_LEAN_AND_MEAN=1)
endif()
