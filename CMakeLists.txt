# Copyright: 2015 BABEL project Plymouth University
# in collaboration with Manchester University
# Author: Samantha Adams
# CopyPolicy: Released under the terms of the FreeBSD license.

cmake_minimum_required(VERSION 2.8)
project(SpinnIO)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")


set(SPINNIO_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/library_src)

# put library under SpinnIO/lib
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
message(STATUS "Library goes to ${LIBRARY_OUTPUT_PATH}")

# add library src dir
add_subdirectory(library_src)





