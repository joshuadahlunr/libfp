cmake_minimum_required(VERSION 3.21)
project(libfp LANGUAGES C CXX)

option(FP_ENABLE_TESTS "Weather or not Unit Tests should be built." ${PROJECT_IS_TOP_LEVEL})
option(FP_FETCH_EXTERNAL_CPPSTL "Weather or not a minimal version of the C++ Standard Template Library should be fetched (Useful for embeded targets)" OFF)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -Wall")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -Wall")

add_library(libfp INTERFACE)
add_library(libfp::fp ALIAS libfp)
target_include_directories(libfp INTERFACE include)

if(${FP_FETCH_EXTERNAL_CPPSTL})
	include(FetchContent)
	FetchContent_Declare(cppstl GIT_REPOSITORY https://github.com/modm-io/avr-libstdcpp.git)
	FetchContent_MakeAvailable(cppstl)

	target_include_directories(libfp INTERFACE ${cppstl_SOURCE_DIR}/include)
	target_compile_definitions(libfp INTERFACE FP_CONSTEXPR=)
endif()

if(${FP_ENABLE_TESTS})
	enable_testing()
	if(TARGET doctest)
	else()
		include(FetchContent) # once in the project to include the module
		FetchContent_Declare(fetch_doctest GIT_REPOSITORY https://github.com/doctest/doctest.git GIT_SHALLOW true)
		FetchContent_MakeAvailable(fetch_doctest)
	endif()
	
	if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
		if(${FP_FETCH_EXTERNAL_CPPSTL})
			add_executable(tst-libfp tests/compiles.c)
		else()
			add_executable(tst-libfp tests/fp.tests.cpp tests/fp.tests.cpp-api.cpp tests/compiles.c)
		endif()
	else()
		add_executable(tst-libfp tests/fp.tests.cpp tests/fp.tests.cpp-api.cpp)
	endif()
	target_link_libraries(tst-libfp PUBLIC doctest libfp)
	set_property(TARGET tst-libfp PROPERTY CXX_STANDARD 23)
	set_property(TARGET tst-libfp PROPERTY C_STANDARD 23)
endif()
