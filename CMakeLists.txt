cmake_minimum_required(VERSION 3.26)
project(share)

set(CMAKE_CXX_STANDARD 20)
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif ()
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

#------- MacOS ------------------------------------------------------
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	find_package(date REQUIRED)
	find_package(range-v3 REQUIRED)
	find_package(TBB REQUIRED)
	find_package(fmt REQUIRED)

	add_subdirectory(csqlite)

	add_library(share STATIC
			share.cpp share.h
			daytime.cpp daytime.h
	)
	target_include_directories(share PUBLIC
			csqlite
			range-v3
			date
	)
	target_link_libraries(share PUBLIC
			csqlite.a
			TBB::tbb TBB::tbbmalloc TBB::tbbmalloc_proxy
			range-v3::meta range-v3::concepts range-v3::range-v3
			fmt
	)
#------- WINDOWS ----------------------------------------------------
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(RANGE-V3-DIR "C:/Users/user/vcpkg/installed/x64-windows")
	find_package(range-v3 PATHS "${RANGE-V3-DIR}/share/range-v3")

	add_subdirectory(csqlite)

	add_library(share STATIC
			datime.cpp datime.h
			share.cpp share.h
	)
	target_link_libraries( share PUBLIC
			csqlite
			range-v3::meta range-v3::concepts range-v3::range-v3
	)
	target_include_directories(share PUBLIC
			${CMAKE_CURRENT_SOURCE_DIR}
			${CMAKE_CURRENT_SOURCE_DIR}/csqlite
			${RANGE-V3-DIR}/include
	)
	#------- LINUX ------------------------------------------------------
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
endif ()

