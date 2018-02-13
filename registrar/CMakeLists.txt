set(REGISTRAR_VALA_FILES
    main.vala
    dbusmenu.vala)
set(REGISTRAR_C_FILES
    registrar-main.c
    registrar-dbusmenu.c)
configure_file (config.h.in config.h)
vala_precompile(VALA_C_REGISTRAR appmenu-registrar
    ${REGISTRAR_VALA_FILES}
    ${CMAKE_BINARY_DIR}/vapi/config.vala
PACKAGES
    ${CORE_PACKAGES}
    gio-addons-2.0
OPTIONS
    --vapidir=${CMAKE_SOURCE_DIR}/vapi
    --vapidir=${CMAKE_CURRENT_BINARY_DIR}
    --vapidir=${CMAKE_BINARY_DIR}/dbusmenu
    --target-glib=2.44
    --gresources=${CMAKE_CURRENT_SOURCE_DIR}/libappmenu.gresource.xml
    --thread
)
add_executable (appmenu-registrar
    ${VALA_C_REGISTRAR}
    ${REGISTRAR_C_FILES}
    ${RESOURCES}
)
target_link_libraries (appmenu-registrar GLIB2::GIO_UNIX GLIB2::GOBJECT m)
target_include_directories (appmenu-registrar PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(appmenu-registrar appmenu-core)
# Install plugin stuffs
install(TARGETS appmenu-registrar DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/vala-panel)
add_custom_target(vala-files-ide-registrar SOURCES ${REGISTRAR_VALA_FILES})

configure_file(${CMAKE_SOURCE_DIR}/data/appmenu-registrar.service.in ${CMAKE_CURRENT_BINARY_DIR}/appmenu-registrar.serivce)
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/appmenu-registrar.serivce DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/dbus-1/services/ RENAME com.canonical.AppMenu.Registrar.service)
