cmake_minimum_required(VERSION 3.20.0)
project(bvh)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
include(CPM)
CPMAddPackage("gh:glfw/glfw#3.4")

add_subdirectory(deps)

if(MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()

if(LINUX)
    find_package(TBB REQUIRED)
    link_libraries(TBB::tbb)
endif()

add_library(viewer viewer.cpp)
target_link_libraries(viewer PUBLIC glad glfw)
target_compile_definitions(viewer PUBLIC GLFW_INCLUDE_NONE)

add_executable(bvh bvh.cpp)
target_link_libraries(bvh PRIVATE viewer)
