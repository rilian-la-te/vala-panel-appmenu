#####
# Core Parts (GResource, definitions)
#####
add_definitions(-fPIC)
add_definitions(-DWNCK_I_KNOW_THIS_IS_UNSTABLE)
set(CORE_VALA_FILES
#    appmenu-bamf.vala
#    appmenu-desktop.vala
#    registrar.vala
#    menubar.vala
    menu-widget.vala
#    menu-widget-dbusmenu.vala
#    menu-widget-menumodel.vala
#    ${CMAKE_BINARY_DIR}/vapi/config.vala
)
vala_precompile(VALA_C_CORE appmenu-core-new
    ${CORE_VALA_FILES}
PACKAGES
    ${CORE_PACKAGES}
    vala-dbusmenu
    gio-addons-2.0
    launcher
    ${WNCK_PACKAGES}
OPTIONS
    --vapidir=${CMAKE_SOURCE_DIR}/vapi
    --vapidir=${CMAKE_CURRENT_BINARY_DIR}
    --vapidir=${CMAKE_BINARY_DIR}/dbusmenu
    --target-glib=2.44
    --gresources=${CMAKE_CURRENT_SOURCE_DIR}/libappmenu.gresource.xml
    --thread
    ${WNCK_DEFINE}
    ${BOLD_DEFINE}
GENERATE_VAPI
    vala-panel-appmenu
)
INCLUDE(GResource)
glib_compile_resources(RESOURCES
    source
        libappmenu.gresource.xml
)
include_directories(${CMAKE_SOURCE_DIR})
#set(CORE_C_FILES launcher.c launcher.h)
add_library(appmenu-core-new STATIC ${CORE_C_FILES} ${VALA_C_CORE} $<TARGET_OBJECTS:vala-dbusmenu-o>)
target_link_libraries (appmenu-core-new ${CORE_LIBRARIES} ${WNCK_LIBRARIES} m)
target_include_directories (appmenu-core-new PRIVATE ${CORE_INCLUDE_DIRS} ${WNCK_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/dbusmenu)
add_custom_target(vala-files-ide-core SOURCES ${VALA_FILES_DBUSMENU} ${CORE_VALA_FILES} ${PLUGIN_VALA_FILES})
