include(CMakeDependentOption)
find_package(Java)
find_package(JNI)
pkg_check_modules(XKBCOMMON
    xkbcommon)
pkg_check_modules(DBUSMENU
    dbusmenu-glib-0.4)
cmake_dependent_option(ENABLE_JAYATANA "Build jayatana fork" OFF
    "JAVA_FOUND;JNI_FOUND;XKBCOMMON_FOUND;DBUSMENU_FOUND" OFF)
if (ENABLE_JAYATANA)
    add_subdirectory(lib)
    add_subdirectory(java)
endif()
