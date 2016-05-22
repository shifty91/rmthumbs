cmake_minimum_required(VERSION 2.6)
project(rmthumbs)

# configure some variables
set(rmthumbs_VERSION "1.2")
configure_file (
  "${PROJECT_SOURCE_DIR}/config.in"
  "${PROJECT_BINARY_DIR}/config.h"
  )
include_directories("${PROJECT_BINARY_DIR}")

# checks
include(CheckFunctionExists)
check_function_exists(getopt HAVE_GETOPT)
check_function_exists(lstat HAVE_LSTAT)

if (NOT HAVE_GETOPT)
  message(FATAL_ERROR "getopt() not found")
endif()
if (NOT HAVE_LSTAT)
  message(FATAL_ERROR "lstat() not found")
endif()

# build
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -pedantic")
set(CMAKE_BUILD_TYPE "Release")
add_executable(rmthumbs rmthumbs.c)

# install
install(TARGETS rmthumbs DESTINATION bin)
