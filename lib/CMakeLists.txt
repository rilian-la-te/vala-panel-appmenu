INCLUDE(GResource)
vala_precompile(VALA_C libappmenu
    registrar.vala
    dbusmenu.vala
    menu-widget-dbusmenu.vala
    menu-widget-menumodel.vala
    ${CMAKE_BINARY_DIR}/vapi/config.vala
PACKAGES
    ${CORE_PACKAGES}
    xcb
    xcb-x11
OPTIONS
    --vapidir=${CMAKE_SOURCE_DIR}/vapi
    --vapidir=${CMAKE_BINARY_DIR}/vapi
    --target-glib=2.40
    --gresources=${CMAKE_CURRENT_SOURCE_DIR}/internal.gresource.xml
    --thread
GENERATE_VAPI
    appmenu
)

add_definitions(-DG_SETTINGS_ENABLE_BACKEND)

#####
# Core Library
#####

# Build library for plugins and application
set (LIBS ${CORE_LIBRARIES} -lm)
set (LIB_PATHS ${CORE_LIBRARY_DIRS})
link_directories (${LIB_PATHS})

set (LIB_FILES ${VALA_C})

add_library (internal STATIC
    ${LIB_FILES}
    ${GLIB_RESOURCES_LIB}
)

target_link_libraries (internal ${LIBS})

set_target_properties (internal PROPERTIES
    VERSION 0.0
    SOVERSION 0)
