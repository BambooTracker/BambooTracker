TARGET = test
LIBS += -lossaudio
DEFINES += __LINUX_OSS__
SOURCES = $$PWD/../../common/test.cpp
