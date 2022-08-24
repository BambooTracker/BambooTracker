cmake_minimum_required (VERSION 3.10)
project (BambooTracker VERSION 0.5.2 LANGUAGES C CXX)

option (INSTALL_FLAT
	"Install everything under CMAKE_INSTALL_PREFIX instead of using Unix-style directory layout (GNUInstallDirs)" ${WIN32}
)

if (NOT INSTALL_FLAT)
	include (GNUInstallDirs)
	set (BT_DATADIR "${CMAKE_INSTALL_DATADIR}/BambooTracker")
	set (BT_ICONDIR "${CMAKE_INSTALL_DATADIR}/icons/hicolor")
	set (BT_LANGDIR "${BT_DATADIR}/lang")
else()
	# Compatibility with QMake default
	if (WIN32 AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		set (CMAKE_INSTALL_PREFIX "C:/BambooTracker" CACHE PATH "..." FORCE)
	endif()

	# Set GNUInstallDirs-style vars to PREFIX
	set (CMAKE_INSTALL_BINDIR "${CMAKE_INSTALL_PREFIX}")
	set (CMAKE_INSTALL_DATADIR "${CMAKE_INSTALL_PREFIX}")
	set (CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_PREFIX}")
	set (CMAKE_INSTALL_MANDIR "${CMAKE_INSTALL_PREFIX}/man")

	set (BT_DATADIR "${CMAKE_INSTALL_PREFIX}")
	set (BT_ICONDIR "${CMAKE_INSTALL_PREFIX}/icons")
	set (BT_LANGDIR "${CMAKE_INSTALL_PREFIX}/lang")
endif()

install (FILES LICENSE DESTINATION "${CMAKE_INSTALL_DOCDIR}")
install (DIRECTORY licenses DESTINATION "${CMAKE_INSTALL_DOCDIR}")

add_subdirectory (submodules)
add_subdirectory (data)
add_subdirectory (BambooTracker)
