set(MODULE_SOURCES
    appmenu-gtk-module.c
    datastructs.c
    datastructs.h
    datastructs-private.h
    hijack.c
    hijack.h
    support.c
    support.h
    blacklist.c
    blacklist.h
    platform.c
    platform.h
    consts.h)

get_filename_component(GTK3_LIBDIR ${GTK3_GTK_LIBRARY} DIRECTORY)
set(GTK3_MODULEDIR ${GTK3_LIBDIR}/gtk-3.0/modules)
add_library(appmenu-gtk3-module MODULE ${MODULE_SOURCES})
target_include_directories(appmenu-gtk3-module PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib ${WAYLAND_INCLUDE})
target_link_libraries(appmenu-gtk3-module PRIVATE appmenu-gtk3-parser GLIB2::GIO_UNIX GTK3::GTK)
set_target_properties(appmenu-gtk3-module PROPERTIES
    OUTPUT_NAME appmenu-gtk-module
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/gtk-3.0)
install(TARGETS appmenu-gtk3-module DESTINATION ${GTK3_MODULEDIR})

if(GTK2_FOUND)
    get_filename_component(GTK2_LIBDIR ${GTK2_GTK_LIBRARY} DIRECTORY)
    set(GTK2_MODULEDIR ${GTK2_LIBDIR}/gtk-2.0/modules)
    add_library(appmenu-gtk2-module MODULE ${MODULE_SOURCES})
    target_include_directories(appmenu-gtk2-module PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib)
    target_link_libraries(appmenu-gtk2-module PRIVATE appmenu-gtk2-parser GLIB2::GIO_UNIX GTK2::GTK)
    set_target_properties(appmenu-gtk2-module PROPERTIES
        OUTPUT_NAME appmenu-gtk-module
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/gtk-2.0)
    install(TARGETS appmenu-gtk2-module DESTINATION ${GTK2_MODULEDIR})
endif()
