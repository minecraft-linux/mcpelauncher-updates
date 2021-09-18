cmake_minimum_required (VERSION 2.6)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_RELEASE} -s")
project(mcpelauncherupdates LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(libjnivm)
add_subdirectory(Webserver)
add_subdirectory(Google-Play-API)
add_library(mcpelauncherupdates SHARED src/main.cpp)
target_link_libraries(mcpelauncherupdates PUBLIC jnivm gplayapi )