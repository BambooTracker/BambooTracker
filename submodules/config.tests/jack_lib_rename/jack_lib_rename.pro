TARGET = test
LIBS += -ljack
DEFINES += __UNIX_JACK__ JACK_HAS_PORT_RENAME
SOURCES = $$PWD/../../common/test.cpp
