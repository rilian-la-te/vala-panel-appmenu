cmake_minimum_required (VERSION 3.3)
cmake_policy (VERSION 3.3)
project (appmenu-registrar C)
enable_testing ()
include(GNUInstallDirs)
include(FeatureSummary)
include(CMakeDependentOption)

list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(REGISTRAR_C_FILES
    registrar-main.c
    registrar-dbusmenu.c)
configure_file (config.h.in config.h)

find_package(GLIB2 2.50 REQUIRED QUIET COMPONENTS GOBJECT GIO GIO_UNIX GENMARSHAL)
include(GLibProgramHandlers)
add_glib_marshal(MARSHALC MARSHALH "registrar-marshal" g_cclosure_user_marshal)

file(STRINGS data/com.canonical.AppMenu.Registrar.xml XML_RAW)
string(CONCAT XML_CONTENTS ${XML_RAW})
file(STRINGS data/org.valapanel.AppMenu.Registrar.xml PRIVATE_RAW)
string(CONCAT PRIVATE_CONTENTS ${PRIVATE_RAW})
configure_file(registrar-xml.c.in registrar-xml.c ESCAPE_QUOTES)

cmake_dependent_option(STANDALONE "Build registrar STANDALONE" ON
        "NOT CORE_FOUND" OFF)

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


add_executable (appmenu-registrar
    ${CMAKE_CURRENT_BINARY_DIR}/registrar-xml.c
    ${REGISTRAR_C_FILES}
    ${MARSHALC}
    ${RESOURCES}
)
target_link_libraries (appmenu-registrar GLIB2::GIO_UNIX GLIB2::GOBJECT m)
# Install plugin stuffs
install(TARGETS appmenu-registrar DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/vala-panel)

configure_file(data/appmenu-registrar.service.in ${CMAKE_CURRENT_BINARY_DIR}/appmenu-registrar.serivce)
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/appmenu-registrar.serivce DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/dbus-1/services/ RENAME com.canonical.AppMenu.Registrar.service)
add_feature_info(Registrar ON "com.canonical.AppMenu.Registrar provider")
