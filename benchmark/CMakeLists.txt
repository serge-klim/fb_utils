#find_package(Boost REQUIRED CONFIG)


add_executable (benchmark
        map_bm.cpp
        avx256.cpp
        )

target_link_libraries(benchmark
        benchmark::benchmark
        fb::fb_utils::simd
        #Boost::boost
        #Boost::disable_autolinking
        )

if (MSVC)
    target_compile_options(benchmark PRIVATE /arch:AVX512)
else()
    target_compile_options(benchmark PRIVATE -mamx-avx512)
endif()