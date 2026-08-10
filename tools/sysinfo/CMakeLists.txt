find_package(Boost COMPONENTS headers REQUIRED )

add_executable (sysinfo
                main.cpp
                sysinfo.cpp
                )

target_link_libraries(sysinfo
        fb::fb_utils
        fb::fb_utils::program_options
        fb::fb_utils::program_options::log
        Boost::boost
        Boost::disable_autolinking
)

target_compile_definitions(sysinfo
    PUBLIC
      $<$<CXX_COMPILER_ID:MSVC>:WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0A00> # Don't include most of Windows.h
  )

