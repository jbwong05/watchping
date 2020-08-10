cmake_minimum_required(VERSION 3.10)

# set the project name
project(4YearPlanner LANGUAGES C)

find_library(RESOLV_LIBRARY NAMES resolv libresolv)
if(NOT RESOLV_LIBRARY)
  message(FATAL_ERROR "resolv library not found")
endif()

add_definitions(-D_GNU_SOURCE)

add_subdirectory(src)