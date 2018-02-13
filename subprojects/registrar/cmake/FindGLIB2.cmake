# - Try to find Glib and its components (gio, gobject etc)
#
# Copyright (C) 2012 Raphael Kubo da Costa <rakuco@webkit.org>
# Copyright (C) 2018 Konstantin Pugin <ria.freelander@gmail.com>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND ITS CONTRIBUTORS ``AS
# IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR ITS
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#defined components:
#GLIB GIO GIO_UNIX GOBJECT GMODULE GTHREAD
set(GLIB2_COMP_LIBRARIES GLIB GIO GIO_UNIX GOBJECT GMODULE GTHREAD)
#GENMARSHAL CODEGEN MKENUMS COMPILE_SCHEMAS COMPILE_RESOURCES GTESTER
set(GLIB2_COMP_TOOLS GENMARSHAL CODEGEN MKENUMS COMPILE_SCHEMAS COMPILE_RESOURCES GTESTER)


INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE(PkgConfig QUIET)
PKG_CHECK_MODULES(PC_GLIB QUIET glib-2.0)

FIND_LIBRARY(GLIB2_GLIB_LIBRARY
    NAMES glib-2.0
    HINTS ${PC_GLIB_LIBDIR}
          ${PC_GLIB_LIBRARY_DIRS}
)

# Files in glib's main include path may include glibconfig.h, which,
# for some odd reason, is normally in $LIBDIR/glib-2.0/include.
GET_FILENAME_COMPONENT(_GLIB_LIBRARY_DIR ${GLIB2_GLIB_LIBRARY} PATH)
FIND_PATH(GLIBCONFIG_INCLUDE_DIR
    NAMES glibconfig.h
    HINTS ${PC_LIBDIR} ${PC_LIBRARY_DIRS} ${_GLIB_LIBRARY_DIR}
    PATH_SUFFIXES glib-2.0/include
)

FIND_PATH(GLIB2_GLIB_INCLUDE
    NAMES glib.h
    HINTS ${PC_GLIB_INCLUDEDIR}
          ${PC_GLIB_INCLUDE_DIRS}
    PATH_SUFFIXES glib-2.0
)

SET(GLIB2_GLIB_INCLUDE_DIRS ${GLIB2_GLIB_INCLUDE} ${GLIBCONFIG_INCLUDE_DIR})

# Version detection
FILE(READ "${GLIBCONFIG_INCLUDE_DIR}/glibconfig.h" GLIBCONFIG_H_CONTENTS)
STRING(REGEX MATCH "#define GLIB_MAJOR_VERSION ([0-9]+)" _dummy "${GLIBCONFIG_H_CONTENTS}")
SET(GLIB_VERSION_MAJOR "${CMAKE_MATCH_1}")
STRING(REGEX MATCH "#define GLIB_MINOR_VERSION ([0-9]+)" _dummy "${GLIBCONFIG_H_CONTENTS}")
SET(GLIB_VERSION_MINOR "${CMAKE_MATCH_1}")
STRING(REGEX MATCH "#define GLIB_MICRO_VERSION ([0-9]+)" _dummy "${GLIBCONFIG_H_CONTENTS}")
SET(GLIB_VERSION_MICRO "${CMAKE_MATCH_1}")
SET(GLIB2_GLIB_VERSION "${GLIB_VERSION_MAJOR}.${GLIB_VERSION_MINOR}.${GLIB_VERSION_MICRO}")
SET(GLIB2_VERSION "${GLIB2_GLIB_VERSION}")

if(GLIB2_GLIB_LIBRARY AND GLIB2_GLIB_INCLUDE_DIRS)
	set(GLIB2_GLIB_FOUND TRUE)
else()
	set(GLIB2_GLIB_FOUND FALSE)
endif()
mark_as_advanced(
	GLIB2_GLIB_LIBRARY
    GLIB2_GLIB_INCLUDE_DIRS
)
if(GLIB2_GLIB_FOUND)
    list(APPEND GLIB2_LIBRARIES
                "${GLIB2_GLIB_LIBRARY}")
    list(APPEND GLIB2_INCLUDE_DIRS
                "${GLIB2_GLIB_INCLUDE_DIRS}")
    set(GLIB2_DEFINITIONS
            ${GLIB2_DEFINITIONS}
            ${PC_GLIB_DEFINITIONS})
    if(NOT TARGET GLIB2::GLIB)
        add_library(GLIB2::GLIB UNKNOWN IMPORTED)
        set_target_properties(GLIB2::GLIB PROPERTIES
            IMPORTED_LOCATION "${GLIB2_GLIB_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${PC_GLIB_DEFINITIONS}"
            INTERFACE_INCLUDE_DIRECTORIES "${GLIB2_GLIB_INCLUDE_DIRS}"
        )
    endif()
    list(APPEND GLIB2_TARGETS
                "GLIB2::GLIB")
endif()

# Additional Glib components.
FOREACH (_component ${GLIB2_FIND_COMPONENTS})
	string(TOLOWER "${_component}" _lc_comp)
	string(REPLACE "_" "-" _rep_comp "${_lc_comp}")
	if(_component IN_LIST GLIB2_COMP_LIBRARIES)
		PKG_CHECK_MODULES(PC_${_component} QUIET ${_rep_comp}-2.0)
		list(APPEND _comp_deps "GLIB2::GLIB")
        list(APPEND _comp_dep_vars "GLIB2_GLIB_FOUND")
		set(_path_suffix glib-2.0)
		set(_library_name ${_rep_comp}-2.0)
	    if (${_component} STREQUAL "GIO_UNIX")
			list(APPEND _comp_deps "GLIB2::GIO")
	        list(APPEND _comp_dep_vars "GLIB2_GIO_FOUND")
			set(_comp_header gio/gdesktopappinfo.h)
			set(_path_suffix gio-unix-2.0)
			set(_library_name gio-2.0)
		elseif (${_component} STREQUAL "GIO")
			list(APPEND _comp_deps "GLIB2::GOBJECT")
	        list(APPEND _comp_dep_vars "GLIB2_GOBJECT_FOUND")
			set(_comp_header gio/gio.h)
			list(APPEND PC_${_component}_DEFINITIONS -DG_SETTINGS_ENABLE_BACKEND)
		elseif (${_component} STREQUAL "GOBJECT")
			set(_comp_header glib-object.h)
		elseif (${_component} STREQUAL "GMODULE")
			set(_comp_header gmodule.h)
		elseif (${_component} STREQUAL "GTHREAD")
			set(_comp_header glib/gthread.h)
		elseif (${_component} STREQUAL "GLIB")
			continue()
		endif()
        find_path(GLIB2_${_component}_INCLUDE_DIR
            NAMES ${_comp_header}
            HINTS ${PC_${_component}_INCLUDE_DIRS}
            PATH_SUFFIXES ${_path_suffix}
        )
        find_library(GLIB2_${_component}_LIBRARY
            NAMES ${_library_name}
            HINTS ${PC_${_component}_LIBRARY_DIRS}
        )
		if(GLIB2_${_component}_LIBRARY AND GLIB2_${_component}_INCLUDE_DIR)
			set(GLIB2_${_component}_FOUND TRUE)
		    foreach(_CURRENT_VAR ${_comp_dep_vars})
				if(NOT ${_CURRENT_VAR})
					set(GLIB2_${_component}_FOUND FALSE)
				endif()
		    endforeach()
		else()
			set(GLIB2_${_component}_FOUND FALSE)
		endif()
        mark_as_advanced(
            GLIB2_${_component}_LIBRARY
            GLIB2_${_component}_INCLUDE_DIR
        )
        if(GLIB2_${_component}_FOUND)
            list(APPEND GLIB2_LIBRARIES
                        "${GLIB2_${_component}_LIBRARY}")
            list(APPEND GLIB2_INCLUDE_DIRS
                        "${GLIB2_${_component}_INCLUDE_DIR}")
            set(GLIB2_DEFINITIONS
                    ${GLIB2_DEFINITIONS}
                    ${PC_${_component}_DEFINITIONS})
            if(NOT TARGET GLIB2::${_component})
                add_library(GLIB2::${_component} UNKNOWN IMPORTED)
                set_target_properties(GLIB2::${_component} PROPERTIES
                    IMPORTED_LOCATION "${GLIB2_${_component}_LIBRARY}"
                    INTERFACE_COMPILE_OPTIONS "${PC_${_component}_DEFINITIONS}"
                    INTERFACE_INCLUDE_DIRECTORIES "${GLIB2_${_component}_INCLUDE_DIR}"
                    INTERFACE_LINK_LIBRARIES "${_comp_deps}"
                )
            endif()
            list(APPEND GLIB2_TARGETS
                        "GLIB2::${_component}")
		endif()
	elseif(_component IN_LIST GLIB2_COMP_TOOLS)
		set(_program_name glib-${_rep_comp})
	    if (${_component} STREQUAL "CODEGEN")
			set(_program_name gdbus-${_rep_comp})
		elseif (${_component} STREQUAL "GTESTER")
			set(_program_name ${_rep_comp})
		endif()

		find_program(GLIB2_${_component}_EXECUTABLE
			${_program_name}
		)
		if(GLIB2_${_component}_EXECUTABLE)
			set(GLIB2_${_component}_FOUND TRUE)
		else()
			set(GLIB2_${_component}_FOUND FALSE)
		endif()
        mark_as_advanced(
            GLIB2_${_component}_EXECUTABLE
        )
        if(GLIB2_${_component}_FOUND)
            if(NOT TARGET GLIB2::${_component})
                add_executable(GLIB2::${_component} IMPORTED)
            endif()
            list(APPEND GLIB2_TARGETS
                        "GLIB2::${_component}")
		endif()
	endif()
ENDFOREACH ()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLIB2
    REQUIRED_VARS
        GLIB2_LIBRARIES
        GLIB2_INCLUDE_DIRS
    HANDLE_COMPONENTS
    VERSION_VAR
        GLIB2_VERSION)


if(GLIB2_LIBRARIES)
    list(REMOVE_DUPLICATES GLIB2_LIBRARIES)
endif()
if(GLIB2_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES GLIB2_INCLUDE_DIRS)
endif()
if(GLIB2_DEFINITIONS)
    list(REMOVE_DUPLICATES GLIB2_DEFINITIONS)
endif()
if(GLIB2_TARGETS)
    list(REMOVE_DUPLICATES GLIB2_TARGETS)
endif()
