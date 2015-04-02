INCLUDE(GResource)
vala_precompile(VALA_C appmenu
    appmenu-bamf.vala
    appmenu-desktop.vala
    registrar.vala
    menubar.vala
    menu-widget-dbusmenu.vala
    menu-widget-menumodel.vala
    ${CMAKE_BINARY_DIR}/vapi/config.vala
    ${XFCE_SOURCES}
PACKAGES
    ${CORE_PACKAGES}
    libbamf3
    libwnck-3.0
    ${XFCE_PACKAGES}
OPTIONS
    --vapidir=${CMAKE_SOURCE_DIR}/vapi
    --vapidir=${CMAKE_BINARY_DIR}/vapi
    --target-glib=2.40
    --gresources=${CMAKE_CURRENT_SOURCE_DIR}/libappmenu.gresource.xml
    --thread
)

add_definitions(-DWNCK_I_KNOW_THIS_IS_UNSTABLE)

#####
# Core Library
#####
INCLUDE(GResource)
glib_compile_resources(RESOURCES
    source
        libappmenu.gresource.xml
)

# Build library for plugins and application
set (LIBS ${CORE_LIBRARIES} ${XFCE_LIBRALIES} -lm)
set (LIB_PATHS ${CORE_LIBRARY_DIRS} ${XFCE_LIBRARY_DIRS})
include_directories (${CORE_INCLUDE_DIRS} ${XFCE_INCLUDE_DIRS})
link_directories (${LIB_PATHS})

set (LIB_FILES ${VALA_C})

add_library (appmenu SHARED
    ${LIB_FILES}
    ${RESOURCES}
)

target_link_libraries (appmenu ${LIBS})

# Install plugin stuffs
install(TARGETS appmenu DESTINATION lib/xfce4/panel/plugins)
