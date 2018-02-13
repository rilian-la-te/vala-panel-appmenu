set(REGISTRAR_C_FILES
    registrar-main.c
    registrar-dbusmenu.c)
configure_file (config.h.in config.h)
find_package(GLIB2 2.50 REQUIRED QUIET COMPONENTS GOBJECT GIO GIO_UNIX GMODULE GTHREAD CODEGEN MKENUMS GENMARSHAL COMPILE_SCHEMAS COMPILE_RESOURCES)
include(GLibProgramHandlers)
add_glib_marshal(MARSHALC MARSHALH "registrar-marshal" g_cclosure_user_marshal)
file(STRINGS com.canonical.AppMenu.Registrar.xml XML_RAW)
string(CONCAT XML_CONTENTS ${XML_RAW})
configure_file(registrar-xml.c.in registrar-xml.c ESCAPE_QUOTES)

add_executable (appmenu-registrar
    ${CMAKE_CURRENT_BINARY_DIR}/registrar-xml.c
    ${REGISTRAR_C_FILES}
    ${MARSHALC}
    ${RESOURCES}
)
target_link_libraries (appmenu-registrar GLIB2::GIO_UNIX GLIB2::GOBJECT m)
target_include_directories (appmenu-registrar PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
# Install plugin stuffs
install(TARGETS appmenu-registrar DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/vala-panel)

configure_file(${CMAKE_SOURCE_DIR}/data/appmenu-registrar.service.in ${CMAKE_CURRENT_BINARY_DIR}/appmenu-registrar.serivce)
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/appmenu-registrar.serivce DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/dbus-1/services/ RENAME com.canonical.AppMenu.Registrar.service)
