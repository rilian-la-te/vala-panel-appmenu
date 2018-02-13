set(GTK_DOC_NAME parser)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${GTK_DOC_NAME}/xml/")
set(ENTITIES_NAME "${CMAKE_CURRENT_BINARY_DIR}/${GTK_DOC_NAME}/xml/gtkdocentities.ent")
set(PACKAGE_BUGREPORT https://github.com/rilian-la-te/vala-panel-appmenu/issues)
set(PACKAGE_URL https://github.com/rilian-la-te/vala-panel-appmenu)
set(PACKAGE_STRING appmenu-gtk-parser)
set(PACKAGE_API_VERSION 2.0)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/gtkdocentities.ent.in ${ENTITIES_NAME})
gtk_doc_add_module(
        ${GTK_DOC_NAME} 
	XML
		appmenu-gtk-parser-docs.sgml
	IGNOREHEADERS
		unity-gtk-menu-shell-private.h		
		unity-gtk-menu-section-private.h	
		unity-gtk-menu-item-private.h		
		unity-gtk-action-group-private.h	
		unity-gtk-action-private.h
        SOURCE
            ${CMAKE_CURRENT_SOURCE_DIR}/../lib
        LIBRARIES
            appmenu-gtk3-parser
)

add_custom_target(documentation ALL DEPENDS doc-parser)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${GTK_DOC_NAME}/html
        DESTINATION ${CMAKE_INSTALL_DOCDIR})
