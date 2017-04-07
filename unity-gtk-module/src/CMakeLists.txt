set(MODULE_SOURCES main.c)

set(GTK3_MODULEDIR ${GTK3_LIBDIR}/gtk-3.0/modules)
add_library(appmenu-gtk3-module MODULE ${MODULE_SOURCES})
target_include_directories(appmenu-gtk3-module PRIVATE ${GLIB_INCLUDE_DIRS} ${GTK3_INCLUDE_DIRS} ${X11_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../lib)
target_link_libraries(appmenu-gtk3-module PRIVATE ${GLIB_LIBRARIES} ${GTK3_LIBRARIES} ${X11_LIBRARIES} appmenu-gtk3-parser)
set_target_properties(appmenu-gtk3-module PROPERTIES
    OUTPUT_NAME appmenu-gtk-module
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/gtk-3.0)
install(TARGETS appmenu-gtk3-module DESTINATION ${GTK3_MODULEDIR} RENAME)

if(GTK2_FOUND)
    set(GTK2_MODULEDIR ${GTK2_LIBDIR}/gtk-2.0/modules)
    add_library(appmenu-gtk2-module MODULE ${MODULE_SOURCES})
    target_include_directories(appmenu-gtk2-module PRIVATE ${GLIB_INCLUDE_DIRS} ${GTK2_INCLUDE_DIRS} ${X11_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../lib)
    target_link_libraries(appmenu-gtk2-module PRIVATE ${GLIB_LIBRARIES} ${GTK2_LIBRARIES} ${X11_LIBRARIES} appmenu-gtk2-parser)
    set_target_properties(appmenu-gtk2-module PROPERTIES
        OUTPUT_NAME appmenu-gtk-module
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/gtk-2.0)
    install(TARGETS appmenu-gtk2-module DESTINATION ${GTK2_MODULEDIR})
endif()
