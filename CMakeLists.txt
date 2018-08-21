cmake_minimum_required (VERSION 3.6 FATAL_ERROR)
cmake_policy (VERSION 3.6)
project (vala-panel-appmenu C)
enable_testing ()
include(GNUInstallDirs)
include(FeatureSummary)
set (DATADIR "${CMAKE_INSTALL_FULL_DATADIR}/vala-panel")
set (PKGDATADIR "${DATADIR}")
set (GETTEXT_PACKAGE "vala-panel-appmenu")
set (RELEASE_NAME "Vala Panel Application Menu")
set (PREFIX ${CMAKE_INSTALL_PREFIX})
list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
if(EXISTS ${CMAKE_SOURCE_DIR}/cmake/VCMSubmodule.cmake)
    include(${CMAKE_SOURCE_DIR}/cmake/VCMSubmodule.cmake)
    use_vcm(${CMAKE_SOURCE_DIR}/cmake)
else()
    find_package(VCM REQUIRED NO_MODULE)
    list(APPEND CMAKE_MODULE_PATH ${VCM_MODULE_PATH})
endif()
include(GitVersion)
determine_fallback_version(${CMAKE_SOURCE_DIR}/cmake)
include(${CMAKE_SOURCE_DIR}/cmake/FallbackVersion.cmake)
include(Format)

# Add 'make dist' command for creating release tarball
set (CPACK_PACKAGE_VERSION ${VERSION})
set (CPACK_PACKAGE_RELEASE "1")
set (CPACK_SOURCE_GENERATOR "TXZ")
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
add_feature_info(LibWnck ENABLE_WNCK "alternative supporting backend")
if (ENABLE_WNCK)
    set(WNCK_DEFINE -D WNCK)
    set(WNCK_PACKAGES libwnck-3.0)
    set(WNCK_TARGETS WNCK::WNCK)
endif()
pkg_check_modules(XFCE libxfce4panel-2.0 libxfconf-0 IMPORTED_TARGET)
cmake_dependent_option(ENABLE_XFCE "Enable Xfce Panel Integration" ON
    "XFCE_FOUND" OFF)
add_feature_info(Xfce ENABLE_XFCE "Xfce Panel Integration")

pkg_check_modules(MATE libmatepanelapplet-4.0>=1.20 IMPORTED_TARGET)
cmake_dependent_option(ENABLE_MATE "Enable mate-panel Integration" ON
    "MATE_FOUND" OFF)
add_feature_info(MATE ENABLE_MATE "MATE Panel Integration")

find_package(PEAS 1.14 COMPONENTS PEAS)
pkg_check_modules(VALAPANEL vala-panel IMPORTED_TARGET)
pkg_check_modules(VP_NEW vala-panel>=0.4.50 QUIET)
pkg_check_modules(BUDGIE budgie-1.0 IMPORTED_TARGET)

cmake_dependent_option(ENABLE_VALAPANEL "Enable Vala Panel Integration" ON
    "VALAPANEL_FOUND" OFF)
if (VP_NEW_FOUND)
    add_feature_info(ValaPanel ENABLE_VALAPANEL "Vala Panel Integration - 0.5.x")
    set(NEW_DEFINE -D NEW)
    set(NEW_IFACE ON)
else()
    add_feature_info(ValaPanel ENABLE_VALAPANEL "Vala Panel Integration - 0.4.x")
endif()

cmake_dependent_option(ENABLE_BUDGIE "Enable budgie-desktop plugin" ON
    "BUDGIE_FOUND; PEAS_FOUND" OFF)
add_feature_info(Budgie ENABLE_BUDGIE "Budgie Desktop Integration")

add_subdirectory(subprojects)
ADD_SUBDIRECTORY(lib)
add_subdirectory(data)
add_subdirectory(po)

feature_summary(WHAT ALL)
