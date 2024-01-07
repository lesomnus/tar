find_package(Catch2 CONFIG REQUIRED)

enable_testing()
include(Catch)



add_custom_target(test-all)

add_custom_target(
	run-test
		COMMAND ${CMAKE_CTEST_COMMAND}
)
add_dependencies(run-test test-all)



if(${PROJECT_IS_TOP_LEVEL} AND CMAKE_BUILD_TYPE STREQUAL "Debug")
	file(
		DOWNLOAD
			https://raw.githubusercontent.com/bilke/cmake-modules/877bab9dd1b17468c5d939cacaa2ad7ba99d1977/CodeCoverage.cmake
	    	${PROJECT_BINARY_DIR}/tests/CodeCoverage.cmake
	)

	SET(HAVE_fprofile_abs_path FALSE) # To avoid clang-tidy unknown compiler argument error.
	include(${PROJECT_BINARY_DIR}/tests/CodeCoverage.cmake)
	append_coverage_compiler_flags_to_target(tar)

	target_link_libraries(
		tar PRIVATE
			gcov
	)

	setup_target_for_coverage_lcov(
		NAME       coverage
		EXECUTABLE ctest --test-dir "${PROJECT_BINARY_DIR}/tests"
		EXCLUDE
			"build/*"
			"tests/*"
			"${CMAKE_INSTALL_PREFIX}/*"
	)
	add_dependencies(coverage test-all)
endif()



macro (TAR_TEST NAME)
	add_executable(
		test-${NAME}
			${NAME}.cpp
	)
	target_link_libraries(
		test-${NAME} PRIVATE
			Catch2::Catch2WithMain
			tar
	)
	catch_discover_tests(
		test-${NAME}
			# EXTRA_ARGS --skip-benchmarks
			# WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	)
	
	add_dependencies(test-all test-${NAME})
endmacro (TAR_TEST)

TAR_TEST(example-simple)
TAR_TEST(marshal)
TAR_TEST(streambuf)
TAR_TEST(string)
TAR_TEST(ustar)
