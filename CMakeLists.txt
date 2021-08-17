cmake_minimum_required(VERSION 3.0)
project(BambooTracker VERSION 0.5.0 LANGUAGES C CXX)

include(GNUInstallDirs)

install(FILES LICENSE DESTINATION "${CMAKE_INSTALL_DOCDIR}")
install(DIRECTORY licenses DESTINATION "${CMAKE_INSTALL_DOCDIR}")

add_subdirectory(submodules)
add_subdirectory(data)
add_subdirectory(BambooTracker)
