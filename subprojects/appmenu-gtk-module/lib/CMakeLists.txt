set(LIB_SOURCES
        appmenu-gtk-action-group.h
	unity-gtk-action-group-private.h
	unity-gtk-action-group.c
	unity-gtk-action-private.h
	unity-gtk-action.c
        appmenu-gtk-menu-shell.h
	unity-gtk-menu-shell-private.h
	unity-gtk-menu-shell.c
	unity-gtk-menu-section-private.h
	unity-gtk-menu-section.c
	unity-gtk-menu-item-private.h
	unity-gtk-menu-item.c
)
set(GTK3_SOURCES
    )
if (STANDALONE)
    set (VERSION_MAJOR "0")
    set (VERSION "0")
endif()

add_library(appmenu-gtk3-parser SHARED
        ${LIB_SOURCES}
        ${GTK3_SOURCES})
target_link_libraries(appmenu-gtk3-parser PRIVATE ${GLIB2_LIBRARIES} ${GTK3_LIBRARIES})
target_include_directories(appmenu-gtk3-parser PRIVATE ${GLIB2_INCLUDE_DIRS} ${GTK3_INCLUDE_DIRS})
set_target_properties(appmenu-gtk3-parser PROPERTIES
	VERSION ${VERSION}
	SOVERSION ${VERSION_MAJOR})
install(TARGETS appmenu-gtk3-parser DESTINATION ${CMAKE_INSTALL_LIBDIR})

if (GTK2_FOUND)
        add_library(appmenu-gtk2-parser SHARED
		${LIB_SOURCES})
	set_target_properties(appmenu-gtk2-parser PROPERTIES
		VERSION ${VERSION}
		SOVERSION ${VERSION_MAJOR})
        target_link_libraries(appmenu-gtk2-parser PRIVATE GLIB2::GIO_UNIX GTK2::GTK)
        install(TARGETS appmenu-gtk2-parser DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

set(LIB_PUBLIC_HEADERS
        appmenu-gtk-parser.h
        appmenu-gtk-menu-shell.h
        appmenu-gtk-action-group.h)

set(GTK_MODULE_INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR}/appmenu-gtk-parser)

install(FILES ${LIB_PUBLIC_HEADERS} DESTINATION ${GTK_MODULE_INCLUDE_DIR})
