find_package(Boost COMPONENTS unit_test_framework program_options REQUIRED )

add_executable (fb_utils_test
		test.cpp
        test_flags.cpp
		test_ini_parser.cpp
		histogram_test.cpp
)

target_link_libraries(fb_utils_test
		fb::fb_utils
		fb::fb_utils::flags
		fb::fb_utils::ini_parser
	    Boost::disable_autolinking
        Boost::boost
		Boost::unit_test_framework
        Boost::program_options
)

target_compile_options(fb_utils_test PRIVATE
     $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
)

add_test(NAME test COMMAND fb_utils_test)
