cmake_minimum_required(VERSION 3.21)


project(
	"tar"
		VERSION   0.1.0
		LANGUAGES CXX
)

option(${PROJECT_NAME}_TIDY  "Run clang-tidy for ${PROJECT_NAME}."           ${PROJECT_IS_TOP_LEVEL})
option(${PROJECT_NAME}_TESTS "Enable ${PROJECT_NAME} project tests targets." ${PROJECT_IS_TOP_LEVEL})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 23)



add_library(
	tar SHARED
		include/tar/detail/marshal.hpp
		include/tar/detail/streambuf.hpp
		include/tar/detail/string.hpp
		include/tar/io.hpp
		include/tar/types.hpp
		include/tar/ustar.hpp
		
		src/marshal.cpp
		src/io.cpp
		src/ustar.cpp
)
target_include_directories(
	tar
		PUBLIC
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
			$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
)



if(${PROJECT_NAME}_TIDY)
	find_program(
		CLANG_TIDY REQUIRED
		NAMES clang-tidy-17 clang-tidy-16 clang-tidy
	)
	set_target_properties(
		tar PROPERTIES
			CXX_CLANG_TIDY ${CLANG_TIDY}
	)
endif()

if(${PROJECT_NAME}_TESTS)
	add_subdirectory(tests)
endif()
