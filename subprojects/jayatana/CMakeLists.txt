cmake_minimum_required (VERSION 3.6 FATAL_ERROR)
cmake_policy (VERSION 3.6)
project (jayatana C)
cmake_policy(SET CMP0054 NEW)
enable_testing ()
include(GNUInstallDirs)
include(CMakeDependentOption)
include(FeatureSummary)
find_package (PkgConfig QUIET)
find_package(Java 1.8)
find_package(JNI)
pkg_check_modules(XKBCOMMON
    xkbcommon
    IMPORTED_TARGET)
pkg_check_modules(DBUSMENU
    dbusmenu-glib-0.4>=16.04.0
    IMPORTED_TARGET)

cmake_dependent_option(STANDALONE "Build jayatana STANDALONE" ON
        "NOT CORE_FOUND;JAVA_FOUND;JNI_FOUND;XKBCOMMON_FOUND;DBUSMENU_FOUND" OFF)
cmake_dependent_option(ENABLE_JAYATANA "Build jayatana fork" OFF
    "JAVA_FOUND;JNI_FOUND;XKBCOMMON_FOUND;DBUSMENU_FOUND" OFF)

add_feature_info(JAyatana ENABLE_JAYATANA "Java support for global menus")

if(STANDALONE)
    find_package(VCM NO_MODULE)
    if(VCM_FOUND)
        list(APPEND CMAKE_MODULE_PATH ${VCM_MODULE_PATH})
        include(GitVersion)
        determine_fallback_version_for_subproject(${CMAKE_SOURCE_DIR}/cmake ${CMAKE_SOURCE_DIR}/../../)
        include(${CMAKE_SOURCE_DIR}/cmake/FallbackVersion.cmake)
        set (CPACK_PACKAGE_VERSION ${VERSION})
        set (CPACK_PACKAGE_RELEASE "1")
    else()
        set (CPACK_PACKAGE_VERSION "unknown")
        set (CPACK_PACKAGE_RELEASE "1")
    endif()
    set (CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION}")
	set (CPACK_SOURCE_IGNORE_FILES "/build/;/.gitmodules;/.git;/.gitignore;~$;${CPACK_SOURCE_IGNORE_FILES}")
	set (CPACK_SOURCE_GENERATOR "TXZ")
	include (CPack)
	add_custom_target (dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
endif()

if (ENABLE_JAYATANA OR STANDALONE)
    add_subdirectory(lib)
    add_subdirectory(java)
endif()
