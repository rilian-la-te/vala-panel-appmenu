cmake_minimum_required (VERSION 3.3)
cmake_policy (VERSION 3.3)
cmake_policy (SET CMP0063 NEW)
project (dbusmenu-importer C)
list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Vala
set(ENUM_FILES
        definitions.h
        )
add_glib_enumtypes(ENUMSC ENUMSH importer-enums ${ENUM_FILES})
add_definitions(-Wall -Wextra
                -Wno-unused-parameter# Vala loves to create variables
                -pedantic
                -Wno-incompatible-pointer-types
                ) #Vala often forgot cast to GtkWidget or gpointer

find_package (PkgConfig)
find_package (PkgConfig)
find_package(GLIB2 2.50 REQUIRED QUIET COMPONENTS GOBJECT GIO GIO_UNIX GMODULE GTHREAD CODEGEN MKENUMS)
include(GLibProgramHandlers)
find_package(GTK 3.22 REQUIRED QUIET COMPONENTS GDK)

pkg_check_modules(PIXBUF REQUIRED QUIET
    gdk-pixbuf-2.0)

set(DBUS_PROXY)
set(DBUS_NAME "dbusmenu-interface")
set(DBUS_NAMESPACE "DBusMenu")
set(XML "com.canonical.dbusmenu.xml")
set(PREFIX "com.canonical")
add_gdbus_codegen(DBUS_GENERATED_SOURCES ${DBUS_NAME} ${PREFIX} ${XML} NAMESPACE ${DBUS_NAMESPACE})
set(DBUSMENU_C_SOURCES
    definitions.h
    item.c
    item.h
    importer.c
    importer.h
    model.h
    model.c
    section.c
    section.h
    utils.c
    utils.h
    debug.c
    debug.h
    )

add_library(dbusmenu-translator-o STATIC
    ${DBUS_GENERATED_SOURCES}
    ${DBUSMENU_C_SOURCES}
    ${ENUMSC}
    ${ENUMSH}
    )
#add_dependencies(dbusmenu-translator-o dbus-xml)
set_target_properties(dbusmenu-translator-o PROPERTIES
#        C_VISIBILITY_PRESET hidden	# Make global variables and functions HIDDEN by default.
        POSITION_INDEPENDENT_CODE ON	# Compile this object code position independent.
)
target_include_directories (dbusmenu-translator-o PRIVATE ${PIXBUF_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dbusmenu-translator-o PRIVATE GLIB2::GIO_UNIX ${PIXBUF_LIBRARIES})


add_executable(dbusmenu-test test.c)
target_link_libraries(dbusmenu-test PRIVATE GLIB2::GIO GTK3::GTK GTK3::GDK dbusmenu-translator-o)
target_include_directories (dbusmenu-test PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})


#xadd_subdirectory(mate-dbusmenu)
