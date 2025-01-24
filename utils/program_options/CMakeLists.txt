find_package( Boost COMPONENTS program_options url log log_setup REQUIRED )

add_library (fb_utils_program_options INTERFACE)

target_sources(fb_utils_program_options PUBLIC FILE_SET HEADERS
                FILES
                 validators/chrono.hpp
                 validators/net.hpp
                 net.hpp
                 chrono.hpp
              )

target_include_directories(fb_utils_program_options INTERFACE
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>
                $<INSTALL_INTERFACE:include/utils/program_options>
              )              

add_library(fb::fb_utils::program_options ALIAS fb_utils_program_options)

target_link_libraries(fb_utils_program_options 
                        INTERFACE 
                        Boost::boost
                        Boost::program_options
                        Boost::url
                        Boost::disable_autolinking
                        )

#target_compile_options(fb_utils_program_options INTERFACE
#     $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
#     $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
#)



add_library (fb_utils_program_options_log
             log.cpp
             console_sink.cpp
)

target_sources(fb_utils_program_options_log PUBLIC FILE_SET HEADERS
                FILES
                log.hpp
                console_sink.hpp
              )

target_include_directories(fb_utils_program_options_log INTERFACE
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>
                $<INSTALL_INTERFACE:include/utils/program_options>
              )              

add_library(fb::fb_utils::program_options::log ALIAS fb_utils_program_options_log)

target_compile_definitions(fb_utils_program_options_log PUBLIC _WIN32_WINNT=0x0602)


target_link_libraries(fb_utils_program_options_log 
                        PUBLIC 
                        Boost::boost
                        Boost::program_options
                        Boost::log
                        Boost::log_setup
                        Boost::disable_autolinking
                        )

target_compile_options(fb_utils_program_options_log PRIVATE
     $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
     $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wpedantic -Werror -Wextra -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-exit-time-destructors>
)


