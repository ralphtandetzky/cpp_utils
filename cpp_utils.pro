QT       -= core gui

QMAKE_CXXFLAGS += -std=c++11 -pedantic

TEMPLATE = lib
CONFIG += staticlib create_prl c++11
DEPENDPATH += .
INCLUDEPATH += ..

# Input
HEADERS += bwt.h \
           cloning.h \
           concurrent_queue.h \
           cow_map.h \
           cow_ptr.h \
           exception.h \
           exception_handling.h \
           extract_by_line.h \
           formula_parser.h \
           locking.h \
           math_constants.h \
           more_algorithms.h \
           optimize.h \
           parallel_executor.h \
           scope_guard.h \
           spin_lock.h \
           std_make_unique.h \
           user_parameter.h \
           user_parameter_container.h \
           value_ptr.h \
           virtual_call.h \
           visitor.h \

SOURCES += bwt.cpp \
           extract_by_line.cpp \
           formula_parser.cpp \
           user_parameter.cpp \
           user_parameter_container.cpp \

LIBS += -L/usr/lib/ -lopencv_core -lopencv_imgproc -lopencv_highgui \


