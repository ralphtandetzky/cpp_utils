#-------------------------------------------------
#
# Project created by QtCreator 2013-07-19T16:31:00
#
#-------------------------------------------------

QT       -= core gui

QMAKE_CXXFLAGS += -std=c++0x

TARGET = cpp_utils
TEMPLATE = lib

DEFINES += CPP_UTILS_LIBRARY

SOURCES +=
	bwt.cpp

HEADERS += \
    std_make_unique.h \
    virtual_call.h \
    sqr.h \
    concurrent_queue.h \
    scope_guard.h \
    parallel_executor.h \
    spin_lock.h \
    locking.h \
    bwt.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
