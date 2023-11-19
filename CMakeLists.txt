cmake_minimum_required(VERSION 3.26)
project(share)

set(CMAKE_CXX_STANDARD 20)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(date REQUIRED)
find_package(range-v3 REQUIRED)
find_package(fmt REQUIRED)
message("--------------------------------------------------------------")
message("range-v3: ${range-v3_DIR} (${range-v3_VERSION})")
message("     fmt: ${fmt_DIR} (${fmt_VERSION})")
message("    date: ${date_DIR} (${date_VERSION})")
message("--------------------------------------------------------------")

add_library(share STATIC
        share.cpp share.h
        daytime.h
)
target_include_directories(share PUBLIC
        range-v3
        date fmt
)
target_link_libraries(share PUBLIC
        fmt::fmt
        date::date date::date-tz
        range-v3::meta range-v3::concepts range-v3::range-v3
)
