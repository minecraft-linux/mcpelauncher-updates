add_executable(WebserverConsole WebserverConsole.cpp)
if(WIN32)
target_link_libraries (WebserverConsole Net)
elseif(UNIX)
set_property(TARGET WebserverConsole PROPERTY INSTALL_RPATH $ORIGIN)
target_link_libraries (WebserverConsole Net stdc++ pthread dl ssl crypto)
endif()

install (TARGETS WebserverConsole DESTINATION bin)
