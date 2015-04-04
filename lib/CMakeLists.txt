#####
# Core Parts (GResource, definitions)
#####
add_definitions(-DWNCK_I_KNOW_THIS_IS_UNSTABLE)
set(CORE_VALA_FILES
    appmenu-bamf.vala
    appmenu-desktop.vala
    registrar.vala
    menubar.vala
    menu-widget.vala
    menu-widget-dbusmenu.vala
    menu-widget-menumodel.vala
)
macro(precompile_core)
    INCLUDE(GResource)
    glib_compile_resources(RESOURCES
        source
            libappmenu.gresource.xml
    )
endmacro(precompile_core)
add_custom_target(appmenu-core DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/libappmenu.gresource.c)
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/vala-panel-appmenu.vapi
   OR ${CMAKE_CURRENT_BINARY_DIR}/libappmenu.gresource.c IS_NEWER_THAN ${CMAKE_CURRENT_BINARY_DIR}/libappmenu.gresource.xml)
   precompile_core()
endif()

##########
# XFCE Part
##########
macro(compile_xfce)
    vala_precompile(VALA_C_XFCE appmenu-xfce
        ${CORE_VALA_FILES}
        ${CMAKE_BINARY_DIR}/vapi/config.vala
        ${XFCE_SOURCES}
    PACKAGES
        ${CORE_PACKAGES}
        ${WNCK_PACKAGES}
        ${XFCE_PACKAGES}
    OPTIONS
        --vapidir=${CMAKE_SOURCE_DIR}/vapi
        --target-glib=2.40
        --gresources=${CMAKE_CURRENT_SOURCE_DIR}/libappmenu.gresource.xml
        --thread
    )

    # Build library for plugins and application
    set (XFCE_LIBS ${CORE_LIBRARIES} ${WNCK_LIBRARIES} ${XFCE_LIBRALIES} -lm)
    set (XFCE_LIB_PATHS ${CORE_LIBRARY_DIRS} ${WNCK_LIBRARY_DIRS} ${XFCE_LIBRARY_DIRS})
    include_directories (${CORE_INCLUDE_DIRS} ${WNCK_INCLUDE_DIRS} ${XFCE_INCLUDE_DIRS})
    link_directories (${XFCE_LIB_PATHS})

    set (XFCE_LIB_FILES ${VALA_C_CORE} ${VALA_C_XFCE})

    add_library (appmenu-xfce SHARED
        ${XFCE_LIB_FILES}
        ${RESOURCES}
    )

    target_link_libraries (appmenu-xfce ${XFCE_LIBS})
    add_dependencies(appmenu-xfce appmenu-core)
    # Install plugin stuffs
    install(TARGETS appmenu-xfce DESTINATION ${XFCE_LOCATION})
endmacro()
if (ENABLE_XFCE)
    compile_xfce()
endif()

##########
# ValaPanel Part
##########
macro(compile_valapanel)
    vala_precompile(VALA_C_VALAPANEL appmenu-valapanel
        ${CORE_VALA_FILES}
        ${VALAPANEL_SOURCES}
    PACKAGES
        ${CORE_PACKAGES}
        ${WNCK_PACKAGES}
        ${VALAPANEL_PACKAGES}
    OPTIONS
        --vapidir=${CMAKE_SOURCE_DIR}/vapi
        --target-glib=2.40
        --gresources=${CMAKE_CURRENT_SOURCE_DIR}/libappmenu.gresource.xml
        --thread
    )

    # Build library for plugins and application
    set (VALAPANEL_LIBS ${CORE_LIBRARIES} ${WNCK_LIBRARIES} ${VALAPANEL_LIBRALIES} -lm)
    set (VALAPANEL_LIB_PATHS ${CORE_LIBRARY_DIRS} ${WNCK_LIBRARY_DIRS} ${VALAPANEL_LIBRARY_DIRS})
    include_directories (${CORE_INCLUDE_DIRS} ${WNCK_INCLUDE_DIRS} ${VALAPANEL_INCLUDE_DIRS})
    link_directories (${VALAPANEL_LIB_PATHS})

    set (VALAPANEL_LIB_FILES ${VALA_C_CORE} ${VALA_C_VALAPANEL})

    add_library (appmenu-valapanel SHARED
        ${VALAPANEL_LIB_FILES}
        ${RESOURCES}
    )
    add_dependencies(appmenu-valapanel appmenu-core)
    target_link_libraries (appmenu-valapanel ${VALAPANEL_LIBS})

    # Install plugin stuffs
    install(TARGETS appmenu-valapanel DESTINATION ${VALAPANEL_LOCATION})
endmacro()
if(ENABLE_VALAPANEL)
    compile_valapanel()
endif()
