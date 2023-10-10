cmake_minimum_required(VERSION 3.26)
project(net)

set(CMAKE_CXX_STANDARD 20)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(RANGE-V3-DIR "C:/Users/user/vcpkg/installed/x64-windows")
find_package(range-v3 PATHS "${RANGE-V3-DIR}/share/range-v3")

add_library(net STATIC
		socket.cpp socket.h
		message.cpp message.h
)
target_include_directories(net PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		${RANGE-V3-DIR}/include
)