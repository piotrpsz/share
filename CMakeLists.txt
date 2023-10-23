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
	find_package(tbb REQUIRED)
	find_package(fmt REQUIRED)
	message("--------------------------------------------------------------")
	message("range-v3: ${range-v3_DIR}")
	message("     fmt: ${fmt_DIR}")
	message("    date: ${date_DIR}")
	message("     tbb: ${tbb_DIR}")
	message("--------------------------------------------------------------")

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
			csqlite
			TBB::tbb TBB::tbbmalloc TBB::tbbmalloc_proxy
			range-v3::meta range-v3::concepts range-v3::range-v3
			fmt
	)
#------- WINDOWS ----------------------------------------------------
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(VCPKG  "C:/Users/user/vcpkg/installed/x64-windows")

	find_package(range-v3 PATHS ${VCPKG})
	find_package(fmt PATHS ${VCPKG})
	find_package(date PATHS ${VCPKG})
#	find_package(unofficial-sqlite3 PATHS ${VCPKG})

	message("--------------------------------------------------------------")
	message("range-v3: ${range-v3_DIR}")
	message("     fmt: ${fmt_DIR}")
	message("    date: ${date_DIR}")
#	message(" sqlite3: ${unofficial-sqlite3_DIR}")
	message("--------------------------------------------------------------")

	add_subdirectory(csqlite)

	add_library(share STATIC
			daytime.cpp daytime.h
			share.cpp share.h
	)
	target_link_libraries( share PRIVATE
			csqlite
			fmt::fmt
			date::date date::date-tz
			range-v3::meta range-v3::concepts range-v3::range-v3
#			unofficial::sqlite3::sqlite3
	)
	target_include_directories(share PRIVATE
			${CMAKE_CURRENT_SOURCE_DIR}
#			${CMAKE_CURRENT_SOURCE_DIR}/csqlite
	)
	#------- LINUX ------------------------------------------------------
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
endif ()

