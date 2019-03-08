#####
# Core Parts (GResource, definitions)
#####
set(CMAKE_INCLUDE_CURRENT_DIR ON)
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
    ${ADDONS_PACKAGE}
    launcher
    ${WNCK_PACKAGES}
OPTIONS
    --vapidir=${CMAKE_SOURCE_DIR}/vapi
    --vapidir=${CMAKE_CURRENT_BINARY_DIR}
    --vapidir=${CMAKE_CURRENT_SOURCE_DIR}/dbusmenu-importer
    --target-glib=2.50
    --gresources=${CMAKE_CURRENT_SOURCE_DIR}/libappmenu.gresource.xml
    ${WNCK_DEFINE}
GENERATE_VAPI
    vala-panel-appmenu
)
include_directories(${CMAKE_SOURCE_DIR})
set(CORE_C_FILES
    launcher.c
    launcher.h
)
if(WNCK_FOUND)
	list(APPEND CORE_C_FILES
		matcher.c
		matcher.h
	)
endif()
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
add_custom_target(vala-files-ide SOURCES ${VALA_FILES_DBUSMENU} ${CORE_VALA_FILES})
