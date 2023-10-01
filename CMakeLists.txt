cmake_minimum_required(VERSION 3.26)
project(share)

set(CMAKE_CXX_STANDARD 20)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(range-v3 PATHS "C:/Users/user/vcpkg/installed/x64-windows/share/range-v3")
add_subdirectory(csqlite)

add_library(share STATIC
        datime.cpp datime.h
        share.cpp share.h
		csqlite
)
target_link_libraries( share PUBLIC
		range-v3::meta range-v3::concepts range-v3::range-v3
)

target_include_directories(share PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
		range-v3
)
