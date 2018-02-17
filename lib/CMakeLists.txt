#####
# Core Parts (GResource, definitions)
#####
add_subdirectory(dbusmenu-importer)
set(CORE_VALA_FILES
    appmenu-abstractions.vala
    appmenu-bamf.vala
    registrar.vala
    menu-widget.vala
    helper-desktop.vala
    helper-dbus.vala
    helper-dbusmenu.vala
    helper-menumodel.vala
    ${CMAKE_BINARY_DIR}/vapi/config.vala
)
vala_precompile(VALA_C_CORE appmenu-core
    ${CORE_VALA_FILES}
PACKAGES
    ${CORE_PACKAGES}
    dbus-menu-importer
    gio-addons-2.0
    launcher
    ${WNCK_PACKAGES}
OPTIONS
    --vapidir=${CMAKE_SOURCE_DIR}/vapi
    --vapidir=${CMAKE_CURRENT_BINARY_DIR}
    --vapidir=${CMAKE_CURRENT_SOURCE_DIR}/dbusmenu-importer
    --target-glib=2.44
    --gresources=${CMAKE_CURRENT_SOURCE_DIR}/libappmenu.gresource.xml
    --thread
    ${WNCK_DEFINE}
GENERATE_VAPI
    vala-panel-appmenu
)
glib_compile_resources(RESOURCES
    source
        libappmenu.gresource.xml
)
include_directories(${CMAKE_SOURCE_DIR})
set(CORE_C_FILES
    launcher.c
    launcher.h
)
set(PLUGIN_VALA_FILES
    valapanel-plugin-appmenu.vala
    xfce4-plugin-appmenu.vala
    mate-plugin-appmenu.vala
    budgie-plugin-appmenu.vala
    )
add_library(appmenu-core STATIC ${CORE_C_FILES} ${VALA_C_CORE})
target_link_libraries (appmenu-core
    dbusmenu-importer
    GLIB2::GIO_UNIX
    GLIB2::GOBJECT
    GTK3::GTK
    GTK3::GDK
    BAMF::LIB
    ${WNCK_TARGETS}
    m)
set_target_properties(appmenu-core PROPERTIES
        POSITION_INDEPENDENT_CODE ON	# Compile this object code position independent.
)
target_include_directories (appmenu-core PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_target(vala-files-ide SOURCES ${VALA_FILES_DBUSMENU} ${CORE_VALA_FILES} ${PLUGIN_VALA_FILES})
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
        --vapidir=${CMAKE_SOURCE_DIR}/vapi
        --vapidir=${CMAKE_CURRENT_BINARY_DIR}
	--target-glib=2.44
        --gresources=${CMAKE_CURRENT_SOURCE_DIR}/libappmenu.gresource.xml
        --thread
        ${WNCK_DEFINE}
    )
    set (XF_CFLAGS ${CORE_CFLAGS} ${WNCK_CFLAGS} ${XFCE_CFLAGS})
    add_definitions(${XF_CFLAGS})
    add_library (appmenu-xfce MODULE
        ${VALA_C_XFCE}
        ${RESOURCES}
    )
    target_link_libraries (appmenu-xfce ${CORE_LIBRARIES} ${WNCK_TARGETS} ${XFCE_LIBRARIES} m appmenu-core)
    target_include_directories (appmenu-xfce PRIVATE ${CORE_INCLUDE_DIRS} ${XFCE_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
    add_dependencies(appmenu-xfce appmenu-core)
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
        --vapidir=${CMAKE_SOURCE_DIR}/vapi
        --vapidir=${CMAKE_CURRENT_BINARY_DIR}
        --vapidir=${CMAKE_BINARY_DIR}/dbusmenu
	--target-glib=2.44
        --gresources=${CMAKE_CURRENT_SOURCE_DIR}/libappmenu.gresource.xml
        --thread
        ${WNCK_DEFINE}
        ${NEW_DEFINE}
    )

    # Build library for plugins and application
    set (VP_CFLAGS ${CORE_CFLAGS} ${WNCK_CFLAGS} ${VALAPANEL_CFLAGS})
    add_definitions(${VP_CFLAGS})
    add_library (appmenu-valapanel MODULE
        ${VALA_C_VALAPANEL}
        ${RESOURCES}
    )
    set_target_properties(appmenu-valapanel PROPERTIES
        OUTPUT_NAME appmenu)
    target_include_directories (appmenu-valapanel PRIVATE ${CORE_INCLUDE_DIRS} ${VALAPANEL_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries (appmenu-valapanel ${CORE_LIBRARIES} ${WNCK_TARGETS} ${VALAPANEL_LIBRARIES} m appmenu-core)
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
        --vapidir=${CMAKE_SOURCE_DIR}/vapi
        --vapidir=${CMAKE_CURRENT_BINARY_DIR}
        --target-glib=2.44
        --gresources=${CMAKE_CURRENT_SOURCE_DIR}/libappmenu.gresource.xml
        --thread
        ${WNCK_DEFINE}
    )

    # Build library for plugins and application
    set (VP_CFLAGS ${CORE_CFLAGS} ${WNCK_CFLAGS} ${BUDGIE_CFLAGS})
    add_definitions(${VP_CFLAGS})
    add_library (appmenu-budgie MODULE
        ${VALA_C_BUDGIE}
        ${RESOURCES}
    )
    add_dependencies(appmenu-budgie appmenu-core)
    target_include_directories (appmenu-budgie PRIVATE ${CORE_INCLUDE_DIRS} ${BUDGIE_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries (appmenu-budgie ${CORE_LIBRARIES} ${WNCK_TARGETS} ${BUDGIE_LIBRARIES} m appmenu-core)
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
	--vapidir=${CMAKE_SOURCE_DIR}/vapi
	--vapidir=${CMAKE_CURRENT_BINARY_DIR}
	--target-glib=2.44
	--gresources=${CMAKE_CURRENT_SOURCE_DIR}/libappmenu.gresource.xml
	--thread
	${WNCK_DEFINE}
    )
    set (MA_CFLAGS ${CORE_CFLAGS} ${WNCK_CFLAGS} ${MATE_CFLAGS})
    add_definitions(${MA_CFLAGS})
    add_executable (appmenu-mate
        ${VALA_C_MATE}
	${RESOURCES}
    )
    target_link_libraries (appmenu-mate ${CORE_LIBRARIES} ${WNCK_TARGETS} ${MATE_LIBRARIES} m appmenu-core)
    target_include_directories (appmenu-mate PRIVATE ${CORE_INCLUDE_DIRS} ${MATE_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
    add_dependencies(appmenu-mate appmenu-core)
    # Install plugin stuffs
    install(TARGETS appmenu-mate DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/mate-panel)
endif()
