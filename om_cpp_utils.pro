# This project underlies the optiMEAS Source Code License which is
# to be found at www.optimeas.de/source_code_license.

QT       -= core gui
TARGET    = om_channels
TEMPLATE  = lib

exists( $$PWD/../../common_config.pri ) : include( $$PWD/../../common_config.pri )

SOURCES +=

HEADERS += \
    algorithm.hpp \
    c++17_features.hpp \
    concurrent.hpp \
    concurrent_queue.hpp \
    cow_ptr.hpp \
    exception.hpp \
    exception_handling.hpp \
    functors.hpp \
    fwd.hpp \
    geometry.hpp \
    json.hpp \
    math_helpers.hpp \
    memory_helpers.hpp \
    meta_functor_binder.hpp \
    meta_programming.hpp \
    monitor.hpp \
    pimpl_ptr.hpp \
    ranges.hpp \
    rank.hpp \
    scope_guard.hpp \
    string_helpers.hpp \
    swap.hpp \
    task_queue.hpp \
    task_queue_thread.hpp \
    units.hpp \
    updater.hpp \
    visitor.hpp \
