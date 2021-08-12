set(RTAUDIO_TARGETNAME_UNINSTALL "uninstall-rtaudio" CACHE STRING "Name of 'uninstall' build target" FORCE)
set(RTAUDIO_BUILD_SHARED_LIBS OFF)

add_subdirectory(src EXCLUDE_FROM_ALL)
