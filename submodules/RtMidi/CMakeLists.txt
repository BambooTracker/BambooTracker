set(RTMIDI_TARGETNAME_UNINSTALL "uninstall-rtmidi" CACHE STRING "Name of 'uninstall' build target" FORCE)
set(RTMIDI_BUILD_SHARED_LIBS OFF)

add_subdirectory(src EXCLUDE_FROM_ALL)
