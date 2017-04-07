set(CLIENT_SOURCES
	altgrabber.h
	altgrabber.c
	altmonitor.h
	altmonitor.c
	menusource.h
	menusource.c
	client.c)

if(ENABLE_WNCK)
	add_executable(menumodel-client ${CLIENT_SOURCES})
	target_include_directories(menumodel-client PRIVATE ${GLIB_INCLUDE_DIRS} ${GTK3_INCLUDE_DIRS} ${WNCK_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../lib)
	target_link_libraries(menumodel-client PRIVATE ${GLIB_LIBRARIES} ${GTK3_LIBRARIES} ${WNCK_LIBRARIES} ${X11_LIBRARIES})
endif()

add_executable(tester unity-gtk-menu-tester.c)
target_include_directories(tester PRIVATE ${GTK3_INCLUDE_DIRS})
target_link_libraries(tester PRIVATE ${GTK3_LIBRARIES})

add_executable(tester-radio test-radio.c)
target_include_directories(tester-radio PRIVATE ${GTK3_INCLUDE_DIRS})
target_link_libraries(tester-radio PRIVATE ${GTK3_LIBRARIES})

add_executable(hello hello.c)
target_include_directories(hello PRIVATE ${GTK3_INCLUDE_DIRS})
target_link_libraries(hello PRIVATE ${GTK3_LIBRARIES})

vala_precompile(VALA_DEMO vala-demo
    black.vala
PACKAGES
    gtk+-3.0
OPTIONS
    --target-glib=2.44
)

add_custom_target(black-ide SOURCES black.vala)
add_executable(blackbox ${CMAKE_CURRENT_BINARY_DIR}/black.c)
target_include_directories(blackbox PRIVATE ${GTK3_INCLUDE_DIRS})
target_link_libraries(blackbox PRIVATE ${GTK3_LIBRARIES})
