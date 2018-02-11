gtk_doc_add_module(
        parser 
	XML
		appmenu-gtk-module-docs.sgml
	IGNOREHEADERS
		unity-gtk-menu-shell-private.h		
		unity-gtk-menu-section-private.h	
		unity-gtk-menu-item-private.h		
		unity-gtk-action-group-private.h	
		unity-gtk-action-private.h
	LIBRARIES
		appmenu-gtk3-parser
    SOURCE
       ${CMAKE_CURRENT_SOURCE_DIR}/../lib
)

add_custom_target(documentation ALL DEPENDS doc-parser)
