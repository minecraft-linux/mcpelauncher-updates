cmake_minimum_required (VERSION 2.6)

project(mcpelauncherupdates LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_library(log-lib log)

add_subdirectory(libjnivm)
add_subdirectory(Webserver)
add_subdirectory(Google-Play-API)
add_library(mcpelauncherupdates SHARED src/main.cpp)
target_link_libraries(mcpelauncherupdates PUBLIC jnivm gplayapi ${log-lib})
