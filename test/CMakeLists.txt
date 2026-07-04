find_package(Boost COMPONENTS unit_test_framework REQUIRED )

add_executable (fb_utils_test
		test.cpp
        test_flags.cpp
		test_net_parsers.cpp
		histogram_test.cpp
)

target_link_libraries(fb_utils_test
		fb::fb_utils
		fb::fb_utils::flags
		fb::fb_utils::program_options
	    Boost::disable_autolinking
        Boost::boost
		Boost::unit_test_framework
)

target_compile_options(fb_utils_test PRIVATE
     $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
)

add_test(NAME test COMMAND fb_utils_test)
