#set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS unit_test_framework REQUIRED )

add_library (fb_utils
             $<$<BOOL:${WIN32}>:win/thread.cpp>
             $<$<NOT:$<BOOL:${WIN32}>>:posix/thread.cpp>
             $<$<BOOL:${WIN32}>:win/sysinfo.cpp>
             $<$<NOT:$<BOOL:${WIN32}>>:posix/sysinfo.cpp>
             $<$<BOOL:${WIN32}>:win/memory.cpp>
             $<$<NOT:$<BOOL:${WIN32}>>:posix/memory.cpp>
             $<$<BOOL:${WIN32}>:win/huge_pages.cpp>
             $<$<NOT:$<BOOL:${WIN32}>>:posix/huge_pages.cpp>
)

target_sources(fb_utils PUBLIC FILE_SET HEADERS
                FILES
                 thread.hpp
                 sysinfo.hpp
                 memory.hpp
                 huge_pages.hpp
                 workarounds.hpp
                 histogram/chrono_axes.hpp
                 $<$<BOOL:${WIN32}>:win/experemental/sysinfo.hpp>
              )

target_include_directories(fb_utils INTERFACE
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
                $<INSTALL_INTERFACE:include/fb_utils>
              )              

add_library(fb::fb_utils ALIAS fb_utils)

target_compile_options(fb_utils PRIVATE
     $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /EHsc>
 #    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror -Wno-c++98-compat>
)

target_compile_definitions(fb_utils PRIVATE
      $<$<CXX_COMPILER_ID:MSVC>:WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0A00> # Don't include most of Windows.h
)


target_link_libraries(fb_utils PUBLIC 
                        Boost::boost
                        Boost::disable_autolinking
                        )



add_library (fb_utils_nic_addresses 
                 $<$<PLATFORM_ID:Windows>:
                        win/nic_addresses.cpp
                 >
                 $<$<NOT:$<PLATFORM_ID:Windows>>:
                        posix/nic_addresses.cpp
                 >
             )

target_sources(fb_utils_nic_addresses PUBLIC FILE_SET HEADERS
                FILES
                detail/nic_addresses.hpp
                nic_addresses.hpp
                 $<$<PLATFORM_ID:Windows>:win/nic_addresses.hpp>
              )

target_include_directories(fb_utils_nic_addresses INTERFACE
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
                $<INSTALL_INTERFACE:include/utils>
              )              

add_library(fb::fb_utils::nic_addresses ALIAS fb_utils_nic_addresses)

target_link_libraries(fb_utils_nic_addresses
             PUBLIC
                $<$<PLATFORM_ID:Windows>:Ws2_32.lib>
                Boost::boost
                Boost::disable_autolinking
            )

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
set(utils_VERSION 0.0.1)


