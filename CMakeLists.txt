cmake_minimum_required (VERSION 2.8)
cmake_policy (VERSION 2.8)
project (vala-panel-appmenu)
enable_testing ()
include(GNUInstallDirs)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")
set (DATADIR "${CMAKE_INSTALL_FULL_DATADIR}/vala-panel")
set (PKGDATADIR "${DATADIR}")
set (GETTEXT_PACKAGE "vala-panel-appmenu")
set (RELEASE_NAME "Vala Panel Application Menu")
set (PREFIX ${CMAKE_INSTALL_PREFIX})
set (DOLLAR "$")
list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

#
# Make a version containing the current version from git.
#

include(GetGitRevisionDescription)
git_describe(VERSION_LONG --tags)

#parse the version information into pieces.
string(REGEX REPLACE "^v*([0-9]+)\\..*" "\\1" VERSION_MAJOR "${VERSION_LONG}")
string(REGEX REPLACE "^v*[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${VERSION_LONG}")
string(REGEX REPLACE "^v*[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${VERSION_LONG}")
string(REGEX REPLACE "^v*[0-9]+\\.[0-9]+\\.[0-9]+(.*)" "\\1" VERSION_SHA1 "${VERSION_LONG}")
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

# Add 'make dist' command for creating release tarball
set (CPACK_PACKAGE_VERSION ${VERSION})
set (CPACK_SOURCE_GENERATOR "TGZ")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION}")
set (CPACK_SOURCE_IGNORE_FILES "/build/;/.gitignore;~$;${CPACK_SOURCE_IGNORE_FILES}")

include (CPack)
add_custom_target (dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

# Configuration
configure_file (${CMAKE_SOURCE_DIR}/vapi/config.vala.in ${CMAKE_BINARY_DIR}/vapi/config.vala)

# Some definitions
add_definitions(-DGETTEXT_PACKAGE=\"${GETTEXT_PACKAGE}\")

# Vala
find_package (Vala REQUIRED)
include (ValaVersion)
ensure_vala_version ("0.24" MINIMUM)
include (ValaPrecompile)

add_definitions(-DVALA_VERSION="${VALA_SHORTVER}")
add_definitions(-w) # Disable gcc warnings

find_package (PkgConfig)

pkg_check_modules (CORE REQUIRED
  gtk+-3.0>=3.10
  glib-2.0>=2.40
  gio-2.0>=2.40
  gio-unix-2.0>=2.40
  gthread-2.0>=2.40
  gmodule-2.0>=2.40
  libbamf3>=0.5.0)

set(CORE_PACKAGES
  gtk+-3.0
  glib-2.0
  gio-2.0
  gio-unix-2.0
  gmodule-2.0
  libbamf3)

#set requred variables
set(TARGET_GLIB ${GLIB_VERSION_MAJOR}.${GLIB_VERSION_MINOR})

#set build options
option(ENABLE_WNCK "Enable libwnck-related functions" ON)
if (ENABLE_WNCK)
    pkg_check_modules(WNCK libwnck-3.0>=3.4.0)
    if (NOT(WNCK_FOUND))
        set(ENABLE_WNCK OFF)
    endif()
endif()
if (ENABLE_WNCK)
    set(WNCK_DEFINE -D WNCK)
    set(WNCK_PACKAGES libwnck-3.0)
endif()
option(ENABLE_XFCE "Enable Xfce Panel Integration" ON)
if (ENABLE_XFCE)
    pkg_check_modules(XFCE libxfce4panel-2.0 libxfconf-0)
    if (NOT(XFCE_FOUND))
        set(ENABLE_XFCE OFF)
    endif()
endif()

option(ENABLE_VALAPANEL "Enable Vala Panel Integration" ON)
if (ENABLE_VALAPANEL)
    pkg_check_modules(VALAPANEL vala-panel libpeas-1.0)
    if (NOT(VALAPANEL_FOUND))
        set(ENABLE_VALAPANEL OFF)
    endif()
endif()

ADD_SUBDIRECTORY(lib)
add_subdirectory(data)
add_subdirectory(po)
