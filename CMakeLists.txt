cmake_minimum_required(VERSION 3.24)

project(fb_utils VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option (BUILD_TESTS "build tests" ON)

add_subdirectory(utils)
add_subdirectory(utils/flags)
add_subdirectory(utils/program_options)

if(BUILD_TESTS)
  include(CTest)
  enable_testing()
  add_subdirectory(test)
endif ()

