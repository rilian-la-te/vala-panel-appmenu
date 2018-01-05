cmake_minimum_required (VERSION 2.8)
cmake_policy (VERSION 2.8)
cmake_policy(SET CMP0054 NEW)
project (vala-panel-appmenu C)
enable_testing ()
include(GNUInstallDirs)
set (DATADIR "${CMAKE_INSTALL_FULL_DATADIR}/vala-panel")
set (PKGDATADIR "${DATADIR}")
set (GETTEXT_PACKAGE "vala-panel-appmenu")
set (RELEASE_NAME "Vala Panel Application Menu")
set (PREFIX ${CMAKE_INSTALL_PREFIX})
set (DOLLAR "$")
list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(GitVersion)
determine_fallback_version(${CMAKE_SOURCE_DIR}/cmake)
include(FallbackVersion)
include(Format)

# Add 'make dist' command for creating release tarball
set (CPACK_PACKAGE_VERSION ${VERSION})
set (CPACK_PACKAGE_RELEASE "1")
set (CPACK_SOURCE_GENERATOR "TGZ")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION}")
set (CPACK_SOURCE_IGNORE_FILES "/build/;/.gitmodules;/.git;/.gitignore;~$;${CPACK_SOURCE_IGNORE_FILES}")

include (CPack)
add_custom_target (dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
#Add RPM build support
include (UseRPMTools)
if (RPMTools_FOUND)
   RPMTools_ADD_RPM_TARGETS(appmenu ${CMAKE_SOURCE_DIR}/rpm/vala-panel-appmenu.spec.in)
   RPMTools_ADD_RPM_TARGETS(module ${CMAKE_SOURCE_DIR}/rpm/appmenu-gtk-module.spec.in)
   RPMTools_ADD_RPM_TARGETS(jayatana ${CMAKE_SOURCE_DIR}/rpm/java-appmenu.spec.in)
endif(RPMTools_FOUND)

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
#add_definitions(-w) #Suppress all GCC warnings
add_definitions(-Wall -Wextra -Wno-unused # Vala loves to create variables
                -Wno-unused-parameter
#		-pedantic
                -Wno-ignored-qualifiers
                -Wno-incompatible-pointer-types
                -Wno-missing-field-initializers
                -Wno-discarded-qualifiers
                -Wno-sign-compare) #Vala often forgot cast to GtkWidget or gpointer

find_package (PkgConfig)

pkg_check_modules (CORE QUIET
  gtk+-3.0>=3.20
  glib-2.0>=2.44
  gio-2.0>=2.44
  gio-unix-2.0>=2.44
  gthread-2.0>=2.44
  gmodule-2.0>=2.44
  libbamf3)

pkg_check_modules(GTK REQUIRED
    gtk+-3.0>=3.20)

pkg_check_modules(GLIB REQUIRED
    glib-2.0>=2.44
    gio-2.0>=2.44
    gio-unix-2.0>=2.44
    gthread-2.0>=2.44
    gmodule-2.0>=2.44)

pkg_check_modules(BAMF REQUIRED
    libbamf3)

set(CORE_PACKAGES
  gtk+-3.0
  glib-2.0
  gio-2.0
  gio-unix-2.0
  gmodule-2.0
  libbamf3
  posix)

#set requred variables
set(TARGET_GLIB ${GLIB_VERSION_MAJOR}.${GLIB_VERSION_MINOR})
include(CMakeDependentOption)

#set build options
pkg_check_modules(WNCK libwnck-3.0>=3.4.0)
cmake_dependent_option(ENABLE_WNCK "Enable libwnck-related functions" ON
    "WNCK_FOUND" OFF)
if (ENABLE_WNCK)
    message("Using libwnck, will be X-dependent")
    set(WNCK_DEFINE -D WNCK)
    set(WNCK_PACKAGES libwnck-3.0)
endif()
pkg_check_modules(XFCE libxfce4panel-2.0 libxfconf-0)
cmake_dependent_option(ENABLE_XFCE "Enable Xfce Panel Integration" ON
    "XFCE_FOUND" OFF)

pkg_check_modules(MATE libmatepanelapplet-4.0)
cmake_dependent_option(ENABLE_MATE "Enable mate-panel Integration" ON
    "MATE_FOUND" OFF)

pkg_check_modules(PEAS libpeas-1.0)
pkg_check_modules(VALAPANEL vala-panel)
pkg_check_modules(VP_NEW vala-panel>=0.3.50 QUIET)
pkg_check_modules(BUDGIE budgie-1.0 libpeas-1.0)

cmake_dependent_option(ENABLE_VALAPANEL "Enable Vala Panel Integration" ON
    "VALAPANEL_FOUND; PEAS_FOUND" OFF)
if (VP_NEW_FOUND)
    message("Using 0.4 ValaPanelApplet api")
    set(NEW_DEFINE -D NEW)
    set(NEW_IFACE ON)
endif()

cmake_dependent_option(ENABLE_BUDGIE "Enable budgie-desktop plugin" ON
    "BUDGIE_FOUND; PEAS_FOUND" OFF)

option(MAKE_BOLD_APPNAME "Make application name bold, if you like it" OFF)
if(MAKE_BOLD_APPNAME)
    set(BOLD_DEFINE -D BOLD)
endif()
add_subdirectory(unity-gtk-module)
ADD_SUBDIRECTORY(jayatana)
add_subdirectory(dbusmenu)
ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(registrar)
add_subdirectory(data)
add_subdirectory(po)
