TARGET = test
CONFIG += link_pkgconfig
PKGCONFIG += libpulse-simple
DEFINES += __LINUX_PULSE__
SOURCES = $$PWD/../../common/test.cpp
