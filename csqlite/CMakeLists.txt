cmake_minimum_required(VERSION 3.26)
project(csqlite)

set(CMAKE_CXX_STANDARD 20)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	find_package(range-v3 REQUIRED)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(RANGE-V3-DIR "C:/Users/user/vcpkg/installed/x64-windows")
	find_package(range-v3 PATHS "${RANGE-V3-DIR}/share/range-v3")
endif ()

add_library(csqlite STATIC
		sqlite.cpp sqlite.h
		stmt.cpp stmt.h
		row.cpp row.h
		result.cpp result.h
		helper.cpp helper.h
        amalgamation/sqlite3.c
		result.cpp
		value.h
		query.h
)

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	target_link_libraries(csqlite PUBLIC
			range-v3::meta range-v3::concepts range-v3::range-v3
	)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
target_include_directories(csqlite PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		${RANGE-V3-DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR/amalgamation}
)
endif ()