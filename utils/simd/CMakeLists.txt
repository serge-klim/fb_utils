add_library (fb_utils_simd INTERFACE)

target_sources(fb_utils_simd PUBLIC FILE_SET HEADERS
                FILES
                 vector.hpp
              )

target_include_directories(fb_utils_simd INTERFACE
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>
                $<INSTALL_INTERFACE:include>
              )              

add_library(fb::fb_utils::simd ALIAS fb_utils_simd)

target_link_libraries(fb_utils_simd)


