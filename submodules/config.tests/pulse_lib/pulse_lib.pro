TARGET = test
LIBS += -lpulse -lpulse-simple
DEFINES += __LINUX_PULSE__
SOURCES = $$PWD/../../common/test.cpp
