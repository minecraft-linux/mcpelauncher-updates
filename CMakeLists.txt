cmake_minimum_required (VERSION 2.6)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_RELEASE} -s")
project(mcpelauncherupdates LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
add_library(mcpelauncherupdates SHARED src/main.cpp)
# add_compile_options(mcpelauncherupdates PUBLIC -Wl,-s)
# add_link_options(mcpelauncherupdates PUBLIC -Wl,-s)
# add_