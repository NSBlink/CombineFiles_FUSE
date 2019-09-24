cmake_minimum_required(VERSION 3.5)
project(CombineFiles_FUSE)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FILE_OFFSET_BITS=64 -fpermissive")
add_definitions(-DFUSE_USE_VERSION=26)
find_package(FUSE REQUIRED)
include_directories(${FUSE_INCLUDE_DIRS})

add_executable(CombineFiles_FUSE main.cpp)
target_link_libraries(CombineFiles_FUSE ${FUSE_LIBRARIES})
