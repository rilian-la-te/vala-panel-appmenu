configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
add_library(jayatanaag SHARED
    com_jarego_jayatana_Agent.h
    com_jarego_jayatana_Agent.c)
target_include_directories(jayatanaag PRIVATE ${JNI_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(jayatanaag PRIVATE ${JNI_LIBRARIES})
pkg_check_modules(LINK REQUIRED QUIET
    glib-2.0
    gio-2.0)
pkg_check_modules(TEMP REQUIRED
    dbusmenu-glib-0.4)
include(UseJava)
add_library(jayatana SHARED
    com_jarego_jayatana_basic_GlobalMenu.c
    com_jarego_jayatana_basic_GMainLoop.c
    com_jarego_jayatana_collections.c
    com_jarego_jayatana_collections.h
    com_jarego_jayatana_jkey2xkey.c
    com_jarego_jayatana_jkey2xkey.h
    com_jarego_jayatana_jni.c
    com_jarego_jayatana_jni.h)
add_dependencies(jayatana java-headers)
target_include_directories(jayatana  PRIVATE ${JNI_INCLUDE_DIRS} ${LINK_INCLUDE_DIRS} ${TEMP_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/../java)
target_link_libraries(jayatana  PRIVATE ${JNI_LIBRARIES} ${LINK_LIBRARIES} ${TEMP_LIBRARIES})

install(TARGETS jayatana jayatanaag DESTINATION ${CMAKE_INSTALL_LIBDIR}/jayatana)
