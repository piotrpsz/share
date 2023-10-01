cmake_minimum_required(VERSION 3.26)
project(csqlite)

set(CMAKE_CXX_STANDARD 20)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_library(csqlite STATIC
		sqlite.cpp sqlite.h
		stmt.cpp stmt.h
		row.cpp row.h
		result.h
		helper.cpp helper.h
        amalgamation/sqlite3.c
)
target_include_directories(csqlite PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR/amalgamation})