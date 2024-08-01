cmake_minimum_required(VERSION 3.21)
project(libfp LANGUAGES C CXX)

option(FP_ENABLE_TESTS "Weather or not Unit Tests should be built." ${PROJECT_IS_TOP_LEVEL})

add_library(libfp INTERFACE)
add_library(libfp::fp ALIAS libfp)
target_include_directories(libfp INTERFACE include)

if(${FP_ENABLE_TESTS})
	enable_testing()
	if(TARGET doctest)
	else()
		include(FetchContent) # once in the project to include the module
		FetchContent_Declare(fetch_doctest GIT_REPOSITORY https://github.com/doctest/doctest.git GIT_SHALLOW true)
		FetchContent_MakeAvailable(fetch_doctest)
	endif()
	
	if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
		add_executable(tst-libfp tests/fp.tests.cpp tests/compiles.c)
	else()
		add_executable(tst-libfp tests/fp.tests.cpp)
	endif()
	target_link_libraries(tst-libfp PUBLIC doctest libfp)
	set_property(TARGET tst-libfp PROPERTY CXX_STANDARD 23)
	set_property(TARGET tst-libfp PROPERTY C_STANDARD 23)
endif()
