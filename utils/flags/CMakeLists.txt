set(Boost_USE_STATIC_LIBS ON)
find_package( Boost REQUIRED )

add_library (fb_utils_flags INTERFACE)

target_sources(fb_utils_flags PUBLIC FILE_SET HEADERS
                FILES
                 parser/flags.hpp
                 flags.hpp
              )

target_include_directories(fb_utils_flags INTERFACE
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>
                $<INSTALL_INTERFACE:include>
              )              

add_library(fb::fb_utils::flags ALIAS fb_utils_flags)

target_link_libraries(fb_utils_flags 
                        INTERFACE 
                        Boost::boost
                        )
