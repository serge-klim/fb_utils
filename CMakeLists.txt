cmake_minimum_required(VERSION 3.24)

project(fb_utils VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option (FB_UTILS_BUILD_TESTS "build tests" OFF)
option (FB_UTILS_BUILD_TOOLS "build sysinfo" OFF)

add_subdirectory(utils)
add_subdirectory(utils/flags)
add_subdirectory(utils/ini_parser)
add_subdirectory(utils/program_options)

if(FB_UTILS_BUILD_TESTS)
  include(CTest)
  enable_testing()
  add_subdirectory(test)
endif (FB_UTILS_BUILD_TESTS)

if(FB_UTILS_BUILD_TOOLS)
  add_subdirectory(tools/sysinfo)
endif (FB_UTILS_BUILD_TOOLS)