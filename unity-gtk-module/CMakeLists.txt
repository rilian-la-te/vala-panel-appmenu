pkg_check_modules(GTK2
	gtk+-2.0>=2.24)
find_package(X11)

cmake_dependent_option(ENABLE_UNITY_GTK_MODULE "Build unity-gtk-module" OFF
	"X11_FOUND" OFF)
cmake_dependent_option(ENABLE_GTK_MODULE_DEMOS "Enable demo programs for gtk module" OFF
	"ENABLE_UNITY_GTK_MODULE" OFF)


if(ENABLE_UNITY_GTK_MODULE)
pkg_check_modules(DOC gtk-doc>=1.26)
	if(DOC_FOUND)
		add_subdirectory(docs)
	endif()
	add_subdirectory(data)
	add_subdirectory(lib)
	add_subdirectory(src)
	add_subdirectory(tests)
endif()
