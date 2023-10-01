cmake_minimum_required(VERSION 3.26)
project(csqlite)
set(CMAKE_CXX_STANDARD 20)

add_library(csqlite STATIC
        sqlite.cpp
        stmt.cpp
        row.cpp
        helper.cpp
        amalgamation/sqlite3.c
)
target_include_directories(csqlite PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR/amalgamation})