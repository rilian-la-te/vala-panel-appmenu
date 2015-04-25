#####
# DBusMenu
#####
macro(precompile_dbusmenu)
vala_precompile(VALA_C_DBUSMENU vala-dbusmenu
    ${CMAKE_SOURCE_DIR}/dbusmenu/dbusmenu.vala
PACKAGES
    gtk+-3.0
    gio-2.0
    glib-2.0
    gobject-2.0
OPTIONS
    --vapidir=${CMAKE_SOURCE_DIR}/vapi
    --target-glib=2.40
    --thread
GENERATE_VAPI
    vala-dbusmenu
GENERATE_HEADER
    vala-dbusmenu
)
endmacro(precompile_dbusmenu)
if(NOT EXISTS ${CMAKE_BINARY_DIR}/vapi/vala-dbusmenu.vapi
   OR ${CMAKE_CURRENT_BINARY_DIR}/vala-dbusmenu_valac.stamp IS_NEWER_THAN ${CMAKE_BINARY_DIR}/vapi/vala-dbusmenu.vapi)
   precompile_dbusmenu()
endif()
add_custom_target(vala-dbusmenu DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/vala-dbusmenu_valac.stamp)
#####
# Core Parts (GResource, definitions)
#####
add_definitions(-DWNCK_I_KNOW_THIS_IS_UNSTABLE)
set(CORE_VALA_FILES
    appmenu-bamf.vala
    appmenu-desktop.vala
    registrar.vala
    menubar.vala
    menu-widget.vala
    menu-widget-dbusmenu.vala
    menu-widget-menumodel.vala
)
macro(precompile_core)
    vala_precompile(VALA_C_CORE appmenu-core
        ${CORE_VALA_FILES}
    PACKAGES
        ${CORE_PACKAGES}
        vala-dbusmenu
        ${WNCK_PACKAGES}
    OPTIONS
        --vapidir=${CMAKE_SOURCE_DIR}/vapi
        --vapidir=${CMAKE_CURRENT_BINARY_DIR}
        --target-glib=2.40
        --gresources=${CMAKE_CURRENT_SOURCE_DIR}/libappmenu.gresource.xml
        --thread
        ${WNCK_DEFINE}
    GENERATE_VAPI
        vala-panel-appmenu
    )
    INCLUDE(GResource)
    glib_compile_resources(RESOURCES
        source
            libappmenu.gresource.xml
    )
endmacro(precompile_core)
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/vala-panel-appmenu.vapi
   OR ${CMAKE_CURRENT_BINARY_DIR}/appmenu-core_valac.stamp IS_NEWER_THAN ${CMAKE_CURRENT_BINARY_DIR}/vala-panel-appmenu.vapi)
   precompile_core()
endif()
add_custom_target(appmenu-core DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/libappmenu.gresource.c ${CMAKE_CURRENT_BINARY_DIR}/appmenu-core_valac.stamp)
add_dependencies(appmenu-core vala-dbusmenu)
##########
# XFCE Part
##########
if (ENABLE_XFCE)
    vala_precompile(VALA_C_XFCE appmenu-xfce
        ${CMAKE_BINARY_DIR}/vapi/config.vala
        xfce4-plugin-appmenu.vala
    PACKAGES
        ${CORE_PACKAGES}
        vala-panel-appmenu
        ${WNCK_PACKAGES}
        libxfce4panel-2.0
        libxfconf-0
    OPTIONS
        --vapidir=${CMAKE_SOURCE_DIR}/vapi
        --vapidir=${CMAKE_CURRENT_BINARY_DIR}
        --target-glib=2.40
        --gresources=${CMAKE_CURRENT_SOURCE_DIR}/libappmenu.gresource.xml
        --thread
        ${WNCK_DEFINE}
    )
    set (XF_CFLAGS ${CORE_CFLAGS} ${WNCK_CFLAGS} ${XFCE_CFLAGS})
    add_definitions(${XF_CFLAGS})
    set (XFCE_LIB_FILES ${VALA_C_CORE} ${VALA_C_XFCE} ${VALA_C_DBUSMENU})
    add_library (appmenu-xfce SHARED
        ${XFCE_LIB_FILES}
        ${RESOURCES}
    )
    target_link_libraries (appmenu-xfce ${CORE_LIBRARIES} ${WNCK_LIBRARIES} ${XFCE_LIBRARIES} m)
    target_include_directories (appmenu-xfce PRIVATE ${CORE_INCLUDE_DIRS} ${WNCK_INCLUDE_DIRS} ${XFCE_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
    link_directories (${CORE_LIBRARY_DIRS} ${WNCK_LIBRARY_DIRS} ${XFCE_LIBRARY_DIRS})
    add_dependencies(appmenu-xfce appmenu-core)
    # Install plugin stuffs
    install(TARGETS appmenu-xfce DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/xfce4/panel/plugins)
endif()

##########
# ValaPanel Part
##########
if(ENABLE_VALAPANEL)
    vala_precompile(VALA_C_VALAPANEL appmenu-valapanel
        valapanel-plugin-appmenu.vala
    PACKAGES
        ${CORE_PACKAGES}
        vala-panel-appmenu
        ${WNCK_PACKAGES}
        vala-panel
        libpeas-1.0
        vala-dbusmenu
    OPTIONS
        --vapidir=${CMAKE_SOURCE_DIR}/vapi
        --vapidir=${CMAKE_CURRENT_BINARY_DIR}
        --target-glib=2.40
        --gresources=${CMAKE_CURRENT_SOURCE_DIR}/libappmenu.gresource.xml
        --thread
        ${WNCK_DEFINE}
    )

    # Build library for plugins and application
    set (VP_CFLAGS ${CORE_CFLAGS} ${WNCK_CFLAGS} ${XFCE_CFLAGS})
    add_definitions(${VP_CFLAGS})
    set (VALAPANEL_LIB_FILES ${VALA_C_CORE} ${VALA_C_VALAPANEL})

    add_library (appmenu-valapanel MODULE
        ${VALAPANEL_LIB_FILES}
        ${RESOURCES}
    )
    add_dependencies(appmenu-valapanel appmenu-core)
    target_include_directories (appmenu-valapanel PRIVATE ${CORE_INCLUDE_DIRS} ${WNCK_INCLUDE_DIRS} ${VALAPANEL_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries (appmenu-valapanel ${CORE_LIBRARIES} ${WNCK_LIBRARIES} ${VALAPANEL_LIBRARIES} m)
    link_directories (${CORE_LIBRARY_DIRS} ${WNCK_LIBRARY_DIRS} ${VALAPANEL_LIBRARY_DIRS})
    # Install plugin stuffs
    install(TARGETS appmenu-valapanel DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/vala-panel/applets)
endif()
