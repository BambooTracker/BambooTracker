TARGET = test
CONFIG += link_pkgconfig
PKGCONFIG += jack
DEFINES += __UNIX_JACK__ JACK_HAS_PORT_RENAME
SOURCES = $$PWD/../../common/test.cpp
