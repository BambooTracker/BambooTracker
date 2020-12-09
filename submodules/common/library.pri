TEMPLATE = lib
CONFIG += staticlib create_prl

CONFIG += c++11

load(configure)
QMAKE_CONFIG_TESTS_DIR = $$PWD/../config.tests
CONFIG += recheck

DESTDIR = $$_PRO_FILE_PWD_
OBJECTS_DIR = $$_PRO_FILE_PWD_
MOC_DIR = $$_PRO_FILE_PWD_
