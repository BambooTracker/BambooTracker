cmake_minimum_required (VERSION 3.10)
project (tst_qtentrypoint LANGUAGES CXX)

set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)
set (CMAKE_AUTOUIC ON)

# Identify Qt version we're using
message (STATUS "Attempting to identify Qt version.")
find_package (Qt6 COMPONENTS Core)
if (Qt6_FOUND)
	set (QT_VERSION 6)
else()
	find_package (Qt5 COMPONENTS Core)
	if (Qt5_FOUND)
		set (QT_VERSION 5)
	else()
		message (FATAL_ERROR "Unable to locate either Qt5 or Qt6!")
	endif()
endif()
message(STATUS "Found Qt${QT_VERSION}")

find_package ("Qt${QT_VERSION}" COMPONENTS Core Gui Widgets REQUIRED)

add_executable (tst_qtentrypoint WIN32
	main.cpp
)

if (QT_VERSION EQUAL 6)
	set (QT_LIBRARIES Qt6::Core Qt6::Gui Qt6::Widgets)
else()
	set (QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)
endif()
target_link_libraries (tst_qtentrypoint PUBLIC ${QT_LIBRARIES})
