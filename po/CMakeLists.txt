configure_file(${CMAKE_SOURCE_DIR}/data/appmenu-mate.desktop.plugin.in ${CMAKE_SOURCE_DIR}/data/appmenu-mate.desktop.in)
include(Translations)
add_translations_directory("vala-panel-appmenu")
add_translations_catalog("vala-panel-appmenu"
    ../lib
    ../data/xfce
)
