 # optiMEAS GmbH Measurement and Automation Systems
 #  ------------------------------------------------
 #
 #  copright (c) 2015 -
 #  author:      $Author$
 #
 #  e-Mail:      info@optimeas.de
 #  web:         http://www.optimeas.de
 #
 #  This file is part of the project:
 #
 #      $Project$
 #      ---------------------------------------------
 #
 # *************************************************************************
 #
 # The source code is given as is. The author is not responsible for any
 # possible damage done due to the use of this code. The component can be
 # freely used in any application. Source code remains property of the
 # author and may not be distributed, published, given or sold in any
 # form as such. No parts of the source code can be included in any other
 # component or application without written authorization of optiMEAS GmbH.
 #
 # The documents version, history and changelog is provided from the
 # version control system (SVN, GIT, etc.). It is never added to the
 # document itself avoiding unnecessary changes and conflicts during
 # merge.
 #
 # ************************************************************************
 #

TEMPLATE = lib
include(../../om_pro_tools/om_pro_tools.pri)

# DO NOT
# - handle dll / staticlib flags
# - overwrite TARGET (it MUST have the same name as the project, (== default))
# - LIBS+= for any om-libraries listed in OM_LIBS
# - INCLUDEPATH+= for ./include or any om-libraries listed in OM_LIBS

CONFIG += console

# what ever you need for Qt ...
QT += core
QT -= gui

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
    c++17_features.hpp \
    concurrent.hpp \
    concurrent_queue.hpp \
    cow_ptr.hpp \
    exception.hpp \
    exception_handling.hpp \
    functors.hpp \
    geometry.hpp \
    math_helpers.hpp \
    memory_helpers.hpp \
    meta_functor_binder.hpp \
    meta_programming.hpp \
    monitor.hpp \
    pimpl_ptr.hpp \
    ranges.hpp \
    scope_guard.hpp \
    string_helpers.hpp \
    swap.hpp \
    task_queue.hpp \
    task_queue_thread.hpp \
    units.hpp \
    updater.hpp \
    visitor.hpp \
    rank.hpp \
    fwd.hpp
