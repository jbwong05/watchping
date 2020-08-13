cmake_minimum_required(VERSION 3.10)

# set the project name
project(watchping LANGUAGES C)

find_library(RESOLV_LIBRARY NAMES resolv libresolv)
if(NOT RESOLV_LIBRARY)
  message(FATAL_ERROR "resolv library not found")
endif()

find_library(NCURSES_LIBRARY NAMES ncurses libncurses)
if(NOT NCURSES_LIBRARY)
  message(FATAL_ERROR "ncurses library not found")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
elseif(CMAKE_BUILD_TYPE STREQUAL Debug)
  add_compile_options(-g)
endif()

if(CMAKE_INSTALL_PREFIX STREQUAL /usr/local)
  set(CMAKE_INSTALL_PREFIX /usr/local/bin)
endif()

add_definitions(-D_GNU_SOURCE)

add_subdirectory(src)