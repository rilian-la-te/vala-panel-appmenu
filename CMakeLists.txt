cmake_minimum_required (VERSION 3.3)
cmake_policy (VERSION 3.3)
project (vala-panel-appmenu C)
enable_testing ()
include(GNUInstallDirs)
set (DATADIR "${CMAKE_INSTALL_FULL_DATADIR}/vala-panel")
set (PKGDATADIR "${DATADIR}")
set (GETTEXT_PACKAGE "vala-panel-appmenu")
set (RELEASE_NAME "Vala Panel Application Menu")
set (PREFIX ${CMAKE_INSTALL_PREFIX})
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
find_package (Vala 0.34 REQUIRED)
include (ValaPrecompile)

add_definitions(-DVALA_VERSION="${VALA_SHORTVER}")
#add_definitions(-w) #Suppress all GCC warnings
add_definitions(-Wall -Wextra -Wno-unused # Vala loves to create variables
                -Wno-unused-parameter
#		-pedantic
                -Wno-ignored-qualifiers
                -Wno-incompatible-pointer-types
                -Wno-missing-field-initializers
                -Wno-sign-compare) #Vala often forgot cast to GtkWidget or gpointer

find_package (PkgConfig)
find_package(GLIB2 2.50 REQUIRED COMPONENTS GOBJECT GIO GIO_UNIX GMODULE GTHREAD CODEGEN MKENUMS GENMARSHAL COMPILE_SCHEMAS COMPILE_RESOURCES)
include(GLibProgramHandlers)
find_package(GTK 3.22 REQUIRED COMPONENTS GDK GTK X11 WAYLAND)
find_package(BAMF 0.5.0 REQUIRED COMPONENTS LIB DAEMON)
if(${GLIB2_FOUND} AND ${GTK_FOUND})
    set(CORE_FOUND TRUE)
endif()

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
find_package(WNCK 3.4.8)
cmake_dependent_option(ENABLE_WNCK "Enable libwnck-related functions" ON
    "WNCK_FOUND" OFF)
if (ENABLE_WNCK)
    message("Using libwnck, will be X-dependent")
    set(WNCK_DEFINE -D WNCK)
    set(WNCK_PACKAGES libwnck-3.0)
    set(WNCK_TARGETS WNCK::WNCK)
endif()
pkg_check_modules(XFCE libxfce4panel-2.0 libxfconf-0)
cmake_dependent_option(ENABLE_XFCE "Enable Xfce Panel Integration" ON
    "XFCE_FOUND" OFF)

pkg_check_modules(MATE libmatepanelapplet-4.0)
cmake_dependent_option(ENABLE_MATE "Enable mate-panel Integration" ON
    "MATE_FOUND" OFF)

find_package(PEAS 1.14 COMPONENTS PEAS)
pkg_check_modules(VALAPANEL vala-panel)
pkg_check_modules(VP_NEW vala-panel>=0.3.50 QUIET)
pkg_check_modules(BUDGIE budgie-1.0)

cmake_dependent_option(ENABLE_VALAPANEL "Enable Vala Panel Integration" ON
    "VALAPANEL_FOUND; PEAS_FOUND" OFF)
if (VP_NEW_FOUND)
    message("Using 0.4 ValaPanelApplet api")
    set(NEW_DEFINE -D NEW)
    set(NEW_IFACE ON)
endif()

cmake_dependent_option(ENABLE_BUDGIE "Enable budgie-desktop plugin" ON
    "BUDGIE_FOUND; PEAS_FOUND" OFF)

add_subdirectory(subprojects)
ADD_SUBDIRECTORY(lib)
add_subdirectory(data)
add_subdirectory(po)
