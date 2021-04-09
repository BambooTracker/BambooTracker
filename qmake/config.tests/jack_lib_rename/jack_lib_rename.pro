TARGET = test
LIBS += -ljack
DEFINES += __UNIX_JACK__ JACK_HAS_PORT_RENAME
SOURCES = $$PWD/../../test.cpp
