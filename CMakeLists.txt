cmake_minimum_required(VERSION 3.0)
project(BambooTracker VERSION 0.5.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

install(FILES LICENSE DESTINATION "${CMAKE_INSTALL_DOCDIR}")
install(DIRECTORY licenses DESTINATION "${CMAKE_INSTALL_DOCDIR}")

set(BUILD_SHARED_LIBS OFF FORCE)
set(BUILD_TESTING OFF FORCE)

add_subdirectory(submodules)
add_subdirectory(data)
add_subdirectory(BambooTracker)
