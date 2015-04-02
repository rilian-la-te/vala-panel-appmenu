cmake_minimum_required (VERSION 2.8)
cmake_policy (VERSION 2.8)
project (vala-panel-appmenu)
enable_testing ()
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set (DATADIR "${CMAKE_INSTALL_PREFIX}/share/vala-panel")
set (PKGDATADIR "${DATADIR}")
set (VERSION "0.1.0")
set (GETTEXT_PACKAGE "vala-panel-appmenu")
set (RELEASE_NAME "Vala Panel Application Menu")
set (PREFIX ${CMAKE_INSTALL_PREFIX})
set (DOLLAR "$")
list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Add 'make dist' command for creating release tarball
set (CPACK_PACKAGE_VERSION ${VERSION})
set (CPACK_SOURCE_GENERATOR "TGZ")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION}")
set (CPACK_SOURCE_IGNORE_FILES "/build/;/.git/;/.gitignore;~$;${CPACK_SOURCE_IGNORE_FILES}")

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
  gtk+-3.0>=3.12
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
  posix)

set(PACKGAGE_CFLAGS ${CORE_CFLAGS})
set(PACKAGE_LINK_DIRS ${CORE_LIBRARY_DIRS})
set(PACKAGE_LIBRARIES ${CORE_LIBRARIES})

add_definitions (${CORE_CFLAGS})
link_libraries (${CORE_LIBRARIES})
link_directories (${CORE_LIBRARY_DIRS})


# Some include paths
include_directories(${CMAKE_BINARY_DIR}/lib)

#set requred variables
set(TARGET_GLIB ${GLIB_VERSION_MAJOR}.${GLIB_VERSION_MINOR})

#set build options
option(ENABLE_WNCK "Enable libwnck-related functions" ON)
if (ENABLE_WNCK)
    pkg_check_modules(WNCK libwnck-3.0>=3.12.0)
    if (NOT(WNCK_FOUND))
        set(ENABLE_WNCK OFF)
    endif()
endif()
if (ENABLE_WNCK)
    set(WNCK_DEFINE -D WNCK)
    set(WNCK_PACKAGES libwnck-3.0)
endif()
option(ENABLE_XFCE "Enable Xfce Panel Integration (Mutually exclusive with Vala Panel)" ON)
if (ENABLE_XFCE)
    pkg_check_modules(XFCE libxfce4panel-2.0 libxfconf-0)
    if (NOT(XFCE_FOUND))
        set(ENABLE_XFCE OFF)
    endif()
endif()
if (ENABLE_XFCE)
    set(ENABLE_VALAPANEL OFF)
    set(XFCE_PACKAGES libxfce4panel-2.0 libxfconf-0)
    set(XFCE_SOURCES dbusmenu.vala xfce4-plugin-appmenu.vala)
    set(XFCE_LOCATION lib/xfce4/panel/plugins)
endif()

option(ENABLE_VALAPANEL "Enable Vala Panel Integration (Mutually exclusive with Xfce Panel)" OFF)
if (ENABLE_VALAPANEL)
    pkg_check_modules(XFCE vala-panel libpeas-1.0)
    if (NOT(XFCE_FOUND))
        set(ENABLE_VALAPANEL OFF)
    endif()
endif()
if (ENABLE_VALAPANEL)
    set(ENABLE_XFCE OFF)
    set(XFCE_PACKAGES vala-panel libpeas-1.0 vala-dbusmenu)
    set(XFCE_SOURCES valapanel-plugin-appmenu.vala)
    set(XFCE_LOCATION lib/vala-panel/applets)
endif()
ADD_SUBDIRECTORY(lib)
add_subdirectory(data)
add_subdirectory(po)
