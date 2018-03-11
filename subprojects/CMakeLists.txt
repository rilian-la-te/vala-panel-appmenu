add_subdirectory(appmenu-gtk-module)
ADD_SUBDIRECTORY(jayatana)
option(ENABLE_REGISTRAR "Enable Appmenu Registrar" ON)

if(ENABLE_REGISTRAR)
    ADD_SUBDIRECTORY(registrar)
endif()
