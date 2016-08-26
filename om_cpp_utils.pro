# This project underlies the optiMEAS Source Code License which is
# to be found at www.optimeas.de/source_code_license.

TEMPLATE = lib
include(../../om_pro_tools/om_pro_tools.pri)

# DO NOT
# - handle dll / staticlib flags
# - overwrite TARGET (it MUST have the same name as the project, (== default))
# - LIBS+= for any om-libraries listed in OM_LIBS
# - INCLUDEPATH+= for ./include or any om-libraries listed in OM_LIBS

CONFIG += console
CONFIG -= qt

# add all prerequisite libraries here as white space list
# e.g. om_log om_scpi ...
# all libraries must be provided in subdirs of ./lib/
# where theLibs name is in the directories path and project 
# file:		
#	lib/<theLib>/<theLib>.pro  
#	=> lib<theLib>.a, <theLib>.dll
#
# NOTE: The traditional behavior of linkers is to search for external functions
# from left to right in the libraries specified on the command line. This means
# that a library containing the definition of a function should appear after any
# source files or object files which use it. This includes libraries specified
# in the OM_LIBS list, too.
OM_LIBS += 

# standards for any build...
DEFINES +=        $$omEnableExports(OM_CPP_UTILS)
# omDependencies()      adds explicitly dependencies to libraries to force static linking if they changed.
# omLibIncludes()	adds the local include and library specific includes from OM_LIBS
# omLibs()              adds the common library pool (-L) and library names (-l) from OM_LIBS
PRE_TARGETDEPS += $$omDependencies()
INCLUDEPATH +=  ..
LIBS +=           $$omLibs()

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
