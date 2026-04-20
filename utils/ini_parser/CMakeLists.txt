add_library (fb_utils_ini_parser
             ini_parser.cpp
)

target_sources(fb_utils_ini_parser PUBLIC FILE_SET HEADERS
                FILES
                 ini_parser.hpp
              )

target_include_directories(fb_utils_ini_parser INTERFACE
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>
                $<INSTALL_INTERFACE:include>
              )              

add_library(fb::fb_utils::ini_parser ALIAS fb_utils_ini_parser)

target_link_libraries(fb_utils_ini_parser)
