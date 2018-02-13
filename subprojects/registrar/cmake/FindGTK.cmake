# - Try to find GTK and its components adn platforms
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
set(GTK_COMP_LIBRARIES GDK GTK UNIX_PRINT)
set(GTK_COMP_TOOLS BUILDER_TOOL ENCODE_SYMBOLIC_SVG)
set(GTK_COMP_PLATFORMS X11 BROADWAY WAYLAND)
set(GTK_VERSION_MAX_SUPPORTED 3)


INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE(PkgConfig QUIET)
#Version checker - sets seachable version
if (${GTK_FIND_VERSION_MAJOR})
	set(_module_name GTK${GTK_FIND_VERSION_MAJOR})
	set(_version_num ${GTK_FIND_VERSION_MAJOR}.0)
	set(_version_short ${GTK_FIND_VERSION_MAJOR})
else()
	set(_module_name GTK${GTK_VERSION_MAX_SUPPORTED})
	set(_version_num ${GTK_VERSION_MAX_SUPPORTED}.0)
	set(_version_short ${GTK_VERSION_MAX_SUPPORTED})
endif()
#Search for GDK and dependencies in include files
PKG_CHECK_MODULES(PC_GDK QUIET gdk-${_version_num})
find_library(${_module_name}_GDK_LIBRARY
    NAMES gdk-${_version_short} gdk-x11-${_version_num}
    HINTS ${PC_GDK_LIBRARY_DIRS}
)
GET_FILENAME_COMPONENT(_GDK_LIBRARY_DIR ${${_module_name}_GDK_LIBRARY} PATH)
FIND_PATH(${_module_name}_GDK_CONFIG_INCLUDE_DIR
    NAMES gdkconfig.h
    HINTS ${PC_GDK_LIBDIR} ${PC_GDK_LIBRARY_DIRS} ${_GDK_LIBRARY_DIR} ${PC_GDK_INCLUDE_DIRS}
    PATH_SUFFIXES gdk-${_version_num}/include gtk-${_version_num}/gdk
)
PKG_CHECK_MODULES(PC_PANGO QUIET pango)
find_path(${_module_name}_INCLUDE_PANGO
    NAMES pango/pango.h
    HINTS ${PC_PANGO_INCLUDE_DIRS}
    PATH_SUFFIXES pango-1.0
)
PKG_CHECK_MODULES(PC_CAIRO QUIET cairo)
find_path(${_module_name}_INCLUDE_CAIRO
    NAMES cairo.h
    HINTS ${PC_CAIRO_INCLUDE_DIRS}
    PATH_SUFFIXES cairo
)
PKG_CHECK_MODULES(PC_PIXBUF QUIET gdk-pixbuf-2.0)
find_path(${_module_name}_INCLUDE_GDK_PIXBUF
    NAMES gdk-pixbuf/gdk-pixbuf.h
    HINTS ${PC_PIXBUF_INCLUDE_DIRS}
    PATH_SUFFIXES gdk-pixbuf-2.0
)
find_path(${_module_name}_INCLUDE_GDK
    NAMES gdk/gdk.h
    HINTS ${PC_GDK_INCLUDE_DIRS}
    PATH_SUFFIXES gdk-${_version_num}
)
set(${_module_name}_GDK_INCLUDE_DIR ${${_module_name}_INCLUDE_GDK}
									  ${${_module_name}_INCLUDE_PANGO}
									  ${${_module_name}_INCLUDE_CAIRO}
									  ${${_module_name}_INCLUDE_GDK_PIXBUF}
									  ${${_module_name}_GDK_CONFIG_INCLUDE_DIR})

if(${_module_name}_GDK_INCLUDE_DIR AND ${_module_name}_GDK_LIBRARY)
	set(${_module_name}_GDK_FOUND TRUE)
else()
	set(${_module_name}_GDK_FOUND FALSE)
endif()
set(GTK_GDK_FOUND ${${_module_name}_GDK_FOUND})

mark_as_advanced(
	${_module_name}_GDK_LIBRARY
    ${_module_name}_GDK_INCLUDE_DIR
)

if(${_module_name}_GDK_FOUND)
    list(APPEND ${_module_name}_LIBRARIES
                "${${_module_name}_GDK_LIBRARY}")
    list(APPEND ${_module_name}_INCLUDE_DIRS
                "${${_module_name}_GDK_INCLUDE_DIR}")
    set(${_module_name}_DEFINITIONS
            ${${_module_name}_DEFINITIONS}
            ${PC_GDK_DEFINITIONS})
    if(NOT TARGET ${_module_name}::GDK)
        add_library(${_module_name}::GDK UNKNOWN IMPORTED)
        set_target_properties(${_module_name}::GDK PROPERTIES
            IMPORTED_LOCATION "${${_module_name}_GDK_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${PC_GDK_DEFINITIONS}"
            INTERFACE_INCLUDE_DIRECTORIES "${${_module_name}_GDK_INCLUDE_DIR}"
        )
    endif()
    list(APPEND ${_module_name}_TARGETS
                "${_module_name}::GDK")
endif()

# Platforms detection
FILE(READ "${${_module_name}_GDK_CONFIG_INCLUDE_DIR}/gdkconfig.h" GDKCONFIG_H_CONTENTS)
foreach(_platform ${GTK_COMP_PLATFORMS})
	STRING(FIND "${GDKCONFIG_H_CONTENTS}" "#define GDK_WINDOWING_${_platform}" _INT)
	if(_INT LESS 0)
		set(${_module_name}_${_platform}_FOUND FALSE)
	else()
		set(${_module_name}_${_platform}_FOUND TRUE)
	endif()
	set(GTK_${_platform}_FOUND ${${_module_name}_${_platform}_FOUND})
endforeach()

#Search for GTK and dependencies in include files

PKG_CHECK_MODULES(PC_${_module_name} QUIET gtk+-${_version_num})

FIND_LIBRARY(${_module_name}_GTK_LIBRARY
    NAMES gtk-${_version_short} gtk-x11-${_version_num}
    HINTS ${PC_${_module_name}_LIBDIR}
          ${PC_${_module_name}_LIBRARY_DIRS}
)

FIND_PATH(${_module_name}_GTK_INCLUDE
    NAMES gtk/gtk.h
    HINTS ${PC_${_module_name}_INCLUDEDIR}
          ${PC_${_module_name}_INCLUDE_DIRS}
    PATH_SUFFIXES gtk-${_version_num}
)
PKG_CHECK_MODULES(PC_ATK QUIET atk-1.0)
FIND_PATH(${_module_name}_ATK_INCLUDE
    NAMES atk/atk.h
    HINTS ${PC_ATK_INCLUDEDIR}
          ${PC_ATK_INCLUDE_DIRS}
    PATH_SUFFIXES atk-1.0
)

# Version detection
FILE(READ "${${_module_name}_GTK_INCLUDE}/gtk/gtkversion.h" GTKVERSION_H_CONTENTS)
STRING(REGEX MATCH "#define GTK_MAJOR_VERSION([ \t]+)\\(([0-9]+)\\)" _dummy "${GTKVERSION_H_CONTENTS}")
SET(${_module_name}_VERSION_MAJOR "${CMAKE_MATCH_2}")
STRING(REGEX MATCH "#define GTK_MINOR_VERSION([ \t]+)\\(([0-9]+)\\)" _dummy "${GTKVERSION_H_CONTENTS}")
SET(${_module_name}_VERSION_MINOR "${CMAKE_MATCH_2}")
STRING(REGEX MATCH "#define GTK_MICRO_VERSION([ \t]+)\\(([0-9]+)\\)" _dummy "${GTKVERSION_H_CONTENTS}")
SET(${_module_name}_VERSION_MICRO "${CMAKE_MATCH_2}")
SET(${_module_name}_VERSION "${${_module_name}_VERSION_MAJOR}.${${_module_name}_VERSION_MINOR}.${${_module_name}_VERSION_MICRO}")
SET(GTK_VERSION "${${_module_name}_VERSION}")

set(${_module_name}_GTK_INCLUDE_DIRS ${${_module_name}_ATK_INCLUDE}
									 ${${_module_name}_GTK_INCLUDE})

if(${_module_name}_GTK_INCLUDE_DIRS AND ${_module_name}_GTK_LIBRARY)
	set(${_module_name}_GTK_FOUND TRUE)
else()
	set(${_module_name}_GTK_FOUND FALSE)
endif()
set(GTK_GTK_FOUND ${${_module_name}_GTK_FOUND})

mark_as_advanced(
	${_module_name}_GTK_LIBRARY
    ${_module_name}_GTK_INCLUDE_DIRS
)
if(${_module_name}_GTK_FOUND)
    list(APPEND ${_module_name}_LIBRARIES
                "${${_module_name}_GTK_LIBRARY}")
    list(APPEND ${_module_name}_INCLUDE_DIRS
                "${${_module_name}_GTK_INCLUDE_DIRS}")
    set(${_module_name}_DEFINITIONS
            ${${_module_name}_DEFINITIONS}
            ${PC_${_module_name}_DEFINITIONS})
    if(NOT TARGET ${_module_name}::GTK)
        add_library(${_module_name}::GTK UNKNOWN IMPORTED)
        set_target_properties(${_module_name}::GTK PROPERTIES
            IMPORTED_LOCATION "${${_module_name}_GTK_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${PC_${_module_name}_DEFINITIONS}"
            INTERFACE_INCLUDE_DIRECTORIES "${${_module_name}_GTK_INCLUDE_DIRS}"
			INTERFACE_LINK_LIBRARIES "${_module_name}::GDK"
        )
    endif()
    list(APPEND ${_module_name}_TARGETS
                "${_module_name}::GTK")
endif()

#Search for unix-print in include files
PKG_CHECK_MODULES(PC_UNIX_PRINT QUIET gtk+-unix-print-${_version_num})
find_path(${_module_name}_UNIX_PRINT_INCLUDE
    NAMES gtk/gtkunixprint.h
    HINTS ${PC_UNIX_PRINT_INCLUDE_DIRS}
    PATH_SUFFIXES gtk-${_version_num}/unix-print
)
if(${_module_name}_UNIX_PRINT_INCLUDE)
	set(${_module_name}_UNIX_PRINT_FOUND TRUE)
else()
	set(${_module_name}_UNIX_PRINT_FOUND FALSE)
endif()
set(GTK_UNIX_PRINT_FOUND ${${_module_name}_UNIX_PRINT_FOUND})

if(${_module_name}_UNIX_PRINT_FOUND)
    list(APPEND ${_module_name}_INCLUDE_DIRS
                "${${_module_name}_UNIX_PRINT_INCLUDE}")
    set(${_module_name}_DEFINITIONS
            ${${_module_name}_DEFINITIONS}
            ${PC_UNIX_PRINT_DEFINITIONS})
    if(NOT TARGET ${_module_name}::UNIX_PRINT)
        add_library(${_module_name}::UNIX_PRINT UNKNOWN IMPORTED)
        set_target_properties(${_module_name}::UNIX_PRINT PROPERTIES
            INTERFACE_COMPILE_OPTIONS "${PC_UNIX_PRINT_DEFINITIONS}"
            INTERFACE_INCLUDE_DIRECTORIES "${${_module_name}_UNIX_PRINT_INCLUDE}"
			INTERFACE_LINK_LIBRARIES "${_module_name}::GTK"
        )
    endif()
    list(APPEND ${_module_name}_TARGETS
                "${_module_name}::UNIX_PRINT")
endif()
#Search for executables
FOREACH (_component ${GTK_COMP_TOOLS})
	string(TOLOWER "${_component}" _lc_comp)
	string(REPLACE "_" "-" _rep_comp "${_lc_comp}")
	set(_program_name gtk-${_rep_comp})
	find_program(${_module_name}_${_component}_EXECUTABLE
		${_program_name}
	)
	if(${_module_name}_${_component}_EXECUTABLE)
		set(${_module_name}_${_component}_FOUND TRUE)
	else()
		set(${_module_name}_${_component}_FOUND FALSE)
	endif()
	set(GTK_${_component}_FOUND ${${_module_name}_${_component}_FOUND})
    mark_as_advanced(
        ${_module_name}_${_component}_EXECUTABLE
    )
    if(${_module_name}_${_component}_FOUND)
        if(NOT TARGET ${_module_name}::${_component})
            add_executable(${_module_name}::${_component} IMPORTED)
        endif()
        list(APPEND ${_module_name}_TARGETS
                    "${_module_name}::${_component}")
	endif()
endforeach()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(GTK
    REQUIRED_VARS
        ${_module_name}_LIBRARIES
        ${_module_name}_INCLUDE_DIRS
	HANDLE_COMPONENTS
    VERSION_VAR
        ${_module_name}_VERSION)

if(${_module_name}_LIBRARIES)
    list(REMOVE_DUPLICATES ${_module_name}_LIBRARIES)
endif()
if(${_module_name}_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES ${_module_name}_INCLUDE_DIRS)
endif()
if(${_module_name}_DEFINITIONS)
    list(REMOVE_DUPLICATES ${_module_name}_DEFINITIONS)
endif()
if(${_module_name}_TARGETS)
    list(REMOVE_DUPLICATES ${_module_name}_TARGETS)
endif()
