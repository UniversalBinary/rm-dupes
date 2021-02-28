cmake_minimum_required(VERSION 3.16)
project(rm-dupes)

set(CMAKE_CXX_STANDARD 20)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.75 COMPONENTS program_options regex filesystem thread chrono REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories("C:\\Program Files\\Boost\\stage\\lib")

add_executable(rm-dupes main.cpp "main.cpp")
target_link_libraries(rm-dupes ${Boost_LIBRARIES})

target_compile_definitions(rm-dupes PRIVATE VERSION=0.90)

find_package(OpenSSL REQUIRED)
target_link_libraries(rm-dupes OpenSSL::SSL)

find_library(GMP_LIBRARY NAMES gmp)
if (GMP_LIBRARY)
    target_link_libraries(rm-dupes ${GMP_LIBRARY})
else()
    message(FATAL_ERROR "libgmp not found!")
endif()

find_package(fmt CONFIG REQUIRED)
target_link_libraries(rm-dupes fmt::fmt)

if (NOT WIN32)
    find_library(TBB_LIBRARY NAMES tbb)
    if (TBB_LIBRARY)
        target_link_libraries(rm-dupes ${TBB_LIBRARY})
    else()
        message(FATAL_ERROR "libtbb not found!")
    endif()

    target_link_libraries(rm-dupes pthread)
else()
    target_compile_definitions(rm-dupes PRIVATE UNICODE=1)
    target_compile_definitions(rm-dupes PRIVATE _UNICODE=1)
    target_compile_definitions(rm-dupes PRIVATE WIN32_LEAN_AND_MEAN=1)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

