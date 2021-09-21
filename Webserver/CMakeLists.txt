cmake_minimum_required (VERSION 2.6)
project (WebServer)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)
find_package(OpenSSL REQUIRED COMPONENTS SSL Crypto)

include_directories(.)
add_subdirectory(Net)
add_subdirectory(WebserverConsole)
