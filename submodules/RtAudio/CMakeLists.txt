set(RTAUDIO_TARGETNAME_UNINSTALL "uninstall-rtaudio" CACHE STRING "Name of 'uninstall' build target" FORCE)
set(RTAUDIO_BUILD_SHARED_LIBS OFF)
set(RTAUDIO_BUILD_TESTING OFF)

add_subdirectory(src EXCLUDE_FROM_ALL)

set (RTAUDIO_INCLUDE_DIRS src PARENT_SCOPE)
set (RTAUDIO_LIBRARIES rtaudio PARENT_SCOPE)
set (RTAUDIO_LDFLAGS rtaudio PARENT_SCOPE)
