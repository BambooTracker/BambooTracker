TARGET = test
LIBS += -lasound
DEFINES += __LINUX_ALSA__
SOURCES = $$PWD/../../common/test.cpp
