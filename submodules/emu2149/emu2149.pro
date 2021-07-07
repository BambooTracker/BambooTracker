include("../../qmake/library.pri")

CONFIG(debug, debug|release):TARGET = emu2149d
else:CONFIG(release, debug|release):TARGET = emu2149

SOURCES = $$PWD/src/emu2149.c
HEADERS = $$PWD/src/emu2149.h
