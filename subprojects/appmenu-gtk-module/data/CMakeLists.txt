add_custom_target(data-files-module SOURCES org.appmenu.gtk-module.gschema.xml appmenu-gtk-module.service)

#----------------------------GSettings macro from CMake repo---------------------------------------
option (GSETTINGS_LOCALINSTALL "Install GSettings Schemas locally instead of to the GLib prefix" ON)

option (GSETTINGS_COMPILE "Compile GSettings Schemas after installation" ${GSETTINGS_LOCALINSTALL})

if(GSETTINGS_LOCALINSTALL)
    message(STATUS "GSettings schemas will be installed locally.")
endif()

if(GSETTINGS_COMPILE)
    message(STATUS "GSettings schemas will be compiled.")
endif()

macro(add_schema SCHEMA_NAME)

    set(PKG_CONFIG_EXECUTABLE pkg-config)
    # Have an option to not install the schema into where GLib is
    SET (GSETTINGS_DIR "${CMAKE_INSTALL_DATAROOTDIR}/glib-2.0/schemas/")
    if (GSETTINGS_LOCALINSTALL)
	SET (GSETTINGS_PREFIX "${CMAKE_INSTALL_PREFIX}")
    else (GSETTINGS_LOCALINSTALL)
	execute_process (COMMAND ${PKG_CONFIG_EXECUTABLE} glib-2.0 --variable prefix OUTPUT_VARIABLE _glib_prefix OUTPUT_STRIP_TRAILING_WHITESPACE)
	SET (GSETTINGS_PREFIX "${_glib_prefix}/share/glib-2.0/schemas/")
    endif (GSETTINGS_LOCALINSTALL)

    # Run the validator and error if it fails
    execute_process (COMMAND ${PKG_CONFIG_EXECUTABLE} gio-2.0 --variable glib_compile_schemas  OUTPUT_VARIABLE _glib_comple_schemas OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process (COMMAND ${_glib_comple_schemas} --dry-run --schema-file=${CMAKE_CURRENT_SOURCE_DIR}/${SCHEMA_NAME} ERROR_VARIABLE _schemas_invalid OUTPUT_STRIP_TRAILING_WHITESPACE)

    if (_schemas_invalid)
      message (SEND_ERROR "Schema validation error: ${_schemas_invalid}")
    endif (_schemas_invalid)

    # Actually install and recomple schemas
    message (STATUS "GSettings schemas will be installed into ${GSETTINGS_PREFIX}/${GSETTINGS_DIR}")
    install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/${SCHEMA_NAME} DESTINATION ${GSETTINGS_DIR} COMPONENT ${ARGV1} OPTIONAL)

    if (GSETTINGS_COMPILE)
	install (CODE "message (STATUS \"Compiling GSettings schemas\")")
	install (CODE "execute_process (COMMAND ${_glib_comple_schemas} ${GSETTINGS_PREFIX}/${GSETTINGS_DIR})")
    endif ()
endmacro()
#---------------------------------------------------endmacro------------------------------------------------------

add_schema("org.appmenu.gtk-module.gschema.xml" "bin")
pkg_check_modules(SYSTEMD "systemd")
if (SYSTEMD_FOUND AND "${SYSTEMD_SERVICES_INSTALL_DIR}" STREQUAL "")
    execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE}
        --variable=systemduserunitdir systemd
       OUTPUT_VARIABLE SYSTEMD_SERVICES_INSTALL_DIR)
    string(REGEX REPLACE "[ \t\n]+" "" SYSTEMD_SERVICES_INSTALL_DIR
        "${SYSTEMD_SERVICES_INSTALL_DIR}")
elseif (NOT SYSTEMD_FOUND AND SYSTEMD_SERVICES_INSTALL_DIR)
    message (FATAL_ERROR "Variable SYSTEMD_SERVICES_INSTALL_DIR is\
        defined, but we can't find systemd using pkg-config")
endif()
configure_file(appmenu-gtk2-parser.pc.in ${CMAKE_CURRENT_BINARY_DIR}/appmenu-gtk2-parser.pc)
configure_file(appmenu-gtk3-parser.pc.in ${CMAKE_CURRENT_BINARY_DIR}/appmenu-gtk3-parser.pc)
configure_file(appmenu-gtk-module.service.in ${CMAKE_CURRENT_BINARY_DIR}/appmenu-gtk-module.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/appmenu-gtk3-parser.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
if(GTK2_FOUND)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/appmenu-gtk2-parser.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()
if (SYSTEMD_FOUND)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/appmenu-gtk-module.service DESTINATION ${SYSTEMD_SERVICES_INSTALL_DIR})
endif()
