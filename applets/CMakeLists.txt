include_directories(${CMAKE_BINARY_DIR}/lib)
set(PLUGIN_VALA_FILES
    valapanel-plugin-appmenu.vala
    xfce4-plugin-appmenu.vala
    mate-plugin-appmenu.vala
    budgie-plugin-appmenu.vala
    )
add_custom_target(plugin-files-ide SOURCES ${PLUGIN_VALA_FILES})
glib_compile_resources(RESOURCES
    source
        ../lib/libappmenu.gresource.xml
)

##########
# XFCE Part
##########
if (ENABLE_XFCE)
    vala_precompile(VALA_C_XFCE appmenu-xfce
        xfce4-plugin-appmenu.vala
    PACKAGES
        ${CORE_PACKAGES}
        vala-panel-appmenu
        ${WNCK_PACKAGES}
        libxfce4panel-2.0
        libxfconf-0
    OPTIONS
        --vapidir=${CMAKE_BINARY_DIR}/lib
        --vapidir=${CMAKE_SOURCE_DIR}/vapi
        --vapidir=${CMAKE_CURRENT_BINARY_DIR}
        --target-glib=2.50
        --gresources=${CMAKE_CURRENT_SOURCE_DIR}/libappmenu.gresource.xml
        ${WNCK_DEFINE}
    )
    add_library (appmenu-xfce MODULE
        ${VALA_C_XFCE}
        ${RESOURCES}
    )
    target_link_libraries (appmenu-xfce appmenu-core PkgConfig::XFCE)
    # Install plugin stuffs
    install(TARGETS appmenu-xfce DESTINATION ${CMAKE_INSTALL_LIBDIR}/xfce4/panel/plugins)
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
    OPTIONS
        --vapidir=${CMAKE_BINARY_DIR}/lib
        --vapidir=${CMAKE_SOURCE_DIR}/vapi
        --vapidir=${CMAKE_CURRENT_BINARY_DIR}
        --vapidir=${CMAKE_BINARY_DIR}/dbusmenu
        --target-glib=2.50
        --gresources=${CMAKE_CURRENT_SOURCE_DIR}/libappmenu.gresource.xml
        ${WNCK_DEFINE}
        ${NEW_DEFINE}
    )

    # Build library for plugins and application
    add_library (appmenu-valapanel MODULE
        ${VALA_C_VALAPANEL}
        ${RESOURCES}
    )
    set_target_properties(appmenu-valapanel PROPERTIES
        OUTPUT_NAME appmenu)
    if(NEW_IFACE)
        target_link_libraries (appmenu-valapanel appmenu-core PkgConfig::VALAPANEL)
    else()
        target_link_libraries (appmenu-valapanel appmenu-core PEAS::PEAS PkgConfig::VALAPANEL)
    endif()
    # Install plugin stuffs
    install(TARGETS appmenu-valapanel DESTINATION ${CMAKE_INSTALL_LIBDIR}/vala-panel/applets)
endif()
##########
# Budgie Panel Part
##########
if(ENABLE_BUDGIE)
    vala_precompile(VALA_C_BUDGIE appmenu-budgie
        budgie-plugin-appmenu.vala
    PACKAGES
        ${CORE_PACKAGES}
        vala-panel-appmenu
        ${WNCK_PACKAGES}
        budgie-1.0
        libpeas-1.0
    OPTIONS
        --vapidir=${CMAKE_BINARY_DIR}/lib
        --vapidir=${CMAKE_SOURCE_DIR}/vapi
        --vapidir=${CMAKE_CURRENT_BINARY_DIR}
        --target-glib=2.50
        --gresources=${CMAKE_CURRENT_SOURCE_DIR}/libappmenu.gresource.xml
        ${WNCK_DEFINE}
    )

    # Build library for plugins and application
    add_library (appmenu-budgie MODULE
        ${VALA_C_BUDGIE}
        ${RESOURCES}
    )
    target_link_libraries (appmenu-budgie appmenu-core PEAS::PEAS PkgConfig::BUDGIE)
    # Install plugin stuffs
    install(TARGETS appmenu-budgie DESTINATION ${CMAKE_INSTALL_LIBDIR}/budgie-desktop/plugins/budgie-vala-panel-appmenu-plugin)
endif()
##########
# MATE Part
##########
if (ENABLE_MATE)
    vala_precompile(VALA_C_MATE appmenu-mate
        mate-plugin-appmenu.vala
    PACKAGES
        ${CORE_PACKAGES}
        vala-panel-appmenu
        ${WNCK_PACKAGES}
        libmatepanelapplet-4.0
    OPTIONS
        --vapidir=${CMAKE_BINARY_DIR}/lib
        --vapidir=${CMAKE_SOURCE_DIR}/vapi
        --vapidir=${CMAKE_CURRENT_BINARY_DIR}
        --target-glib=2.50
        --gresources=${CMAKE_CURRENT_SOURCE_DIR}/libappmenu.gresource.xml
        ${WNCK_DEFINE}
    )
    add_library (appmenu-mate MODULE
                ${VALA_C_MATE}
                ${RESOURCES}
    )
    target_link_libraries (appmenu-mate appmenu-core PkgConfig::MATE)
    # Install plugin stuffs
    install(TARGETS appmenu-mate DESTINATION ${CMAKE_INSTALL_LIBDIR}/mate-panel)
endif()
