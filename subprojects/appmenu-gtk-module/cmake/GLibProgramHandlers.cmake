INCLUDE(CMakeParseArguments)
find_package(GLIB2 QUIET COMPONENTS CODEGEN MKENUMS GENMARSHAL COMPILE_SCHEMAS COMPILE_RESOURCES GTESTER)

option (GSETTINGS_LOCALINSTALL "Install GSettings Schemas locally instead of to the GLib prefix" ON)

option (GSETTINGS_COMPILE "Compile GSettings Schemas after installation" ${GSETTINGS_LOCALINSTALL})

macro(add_schema SCHEMA_NAME)
    set(PKG_CONFIG_EXECUTABLE pkg-config)
    # Have an option to not install the schema into where GLib is
    SET (GSETTINGS_DIR "${CMAKE_INSTALL_DATAROOTDIR}/glib-2.0/schemas/")
    if (GSETTINGS_LOCALINSTALL)
        SET (GSETTINGS_PREFIX "${CMAKE_INSTALL_PREFIX}")
    else ()
        execute_process (COMMAND ${PKG_CONFIG_EXECUTABLE} glib-2.0 --variable prefix OUTPUT_VARIABLE _glib_prefix OUTPUT_STRIP_TRAILING_WHITESPACE)
        SET (GSETTINGS_PREFIX "${_glib_prefix}")
    endif ()

    # Run the validator and error if it fails
    execute_process (COMMAND ${GLIB2_COMPILE_SCHEMAS_EXECUTABLE} --dry-run --schema-file=${CMAKE_CURRENT_SOURCE_DIR}/${SCHEMA_NAME} ERROR_VARIABLE _schemas_invalid OUTPUT_STRIP_TRAILING_WHITESPACE)

    if (_schemas_invalid)
      message (SEND_ERROR "Schema validation error: ${_schemas_invalid}")
    endif (_schemas_invalid)

    # Actually install and recomple schemas
    message (STATUS "GSettings schemas will be installed into ${GSETTINGS_PREFIX}/${GSETTINGS_DIR}")
    install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/${SCHEMA_NAME} DESTINATION ${GSETTINGS_PREFIX}/${GSETTINGS_DIR} COMPONENT ${ARGV1} OPTIONAL)

    if (GSETTINGS_COMPILE)
        install (CODE "message (STATUS \"Compiling GSettings schemas\")")
        install (CODE "execute_process (COMMAND ${GLIB2_COMPILE_SCHEMAS_EXECUTABLE} ${GSETTINGS_PREFIX}/${GSETTINGS_DIR})")
    endif ()
endmacro()

macro(add_glib_marshal outsources outincludes name prefix)
  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name}.h"
    COMMAND ${GLIB2_GENMARSHAL_EXECUTABLE} --header "--prefix=${prefix}"
            "${CMAKE_CURRENT_SOURCE_DIR}/${name}.list"
            > "${CMAKE_CURRENT_BINARY_DIR}/${name}.h"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${name}.list"
  )
  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name}.c"
    COMMAND ${GLIB2_GENMARSHAL_EXECUTABLE} --body "--prefix=${prefix}"
            "${CMAKE_CURRENT_SOURCE_DIR}/${name}.list"
            > "${CMAKE_CURRENT_BINARY_DIR}/${name}.c"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${name}.list"
            "${CMAKE_CURRENT_BINARY_DIR}/${name}.h"
  )
  list(APPEND ${outsources} "${CMAKE_CURRENT_BINARY_DIR}/${name}.c")
  list(APPEND ${outincludes} "${CMAKE_CURRENT_BINARY_DIR}/${name}.h")
endmacro(add_glib_marshal)

macro(add_glib_enumtypes outsources outheaders name)
    set(files ${ARGN})
	add_custom_command(
	  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name}.h"
	  COMMAND ${GLIB2_MKENUMS_EXECUTABLE} ARGS --template ${name}".h.template"
          ${files}
		  > "${CMAKE_CURRENT_BINARY_DIR}/${name}.h"
	  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${name}.h.template"
		${files}
	)
	add_custom_command(
		OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name}.c"
		COMMAND ${GLIB2_MKENUMS_EXECUTABLE} ARGS --template ${name}".c.template"
                ${files}
			> "${CMAKE_CURRENT_BINARY_DIR}/${name}.c"
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${name}.c.template"
			${files}
	)
	list(APPEND ${outsources} "${CMAKE_CURRENT_BINARY_DIR}/${name}.c")
	list(APPEND ${outheaders} "${CMAKE_CURRENT_BINARY_DIR}/${name}.h")
endmacro(add_glib_enumtypes)
#.rst:
#.. command:: add_gdbus_codegen
#
#  Generates C code and header file from XML service description, and
#  appends the sources to the SOURCES list provided.
#
#    add_gdbus_codegen(<SOURCES> <NAME> <PREFIX> <SERVICE_XML> [NAMESPACE])
#
#  For example:
#
#  .. code-block:: cmake
#
#   set(MY_SOURCES foo.c)
#
#   add_gdbus_codegen(MY_SOURCES
#     dbus-proxy
#     org.freedesktop
#     org.freedesktop.DBus.xml
#     )
#
function(ADD_GDBUS_CODEGEN _SOURCES _NAME _PREFIX SERVICE_XML)
  set(_options ALL)
  set(_oneValueArgs NAMESPACE)

  cmake_parse_arguments(_ARG "${_options}" "${_oneValueArgs}" "" ${ARGN})

  get_filename_component(_ABS_SERVICE_XML ${SERVICE_XML} ABSOLUTE)

  set(_NAMESPACE "")
  if(_ARG_NAMESPACE)
    set(_NAMESPACE "--c-namespace=${_ARG_NAMESPACE}")
  endif()

  set(_OUTPUT_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/${_NAME}")
  set(_OUTPUT_FILES "${_OUTPUT_PREFIX}.c" "${_OUTPUT_PREFIX}.h")

  # for backwards compatibility
  set("${_SOURCES}_SOURCES" "${_OUTPUT_FILES}" PARENT_SCOPE)


  list(APPEND ${_SOURCES} ${_OUTPUT_FILES})
  set(${_SOURCES} ${${_SOURCES}} PARENT_SCOPE)

  add_custom_command(
    OUTPUT ${_OUTPUT_FILES}
    COMMAND ${GLIB2_CODEGEN_EXECUTABLE}
        --interface-prefix ${_PREFIX}
        --generate-c-code="${_NAME}"
        --c-generate-autocleanup=all
        ${_NAMESPACE}
        ${_ABS_SERVICE_XML}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${_ABS_SERVICE_XML}
    )
endfunction()

FUNCTION(GLIB_COMPILE_RESOURCES output)
  CMAKE_PARSE_ARGUMENTS(ARGS "" "SOURCE" ${ARGN})
  SET(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  SET(out_files "")

  FOREACH(src ${ARGS_SOURCE} ${ARGS_UNPARSED_ARGUMENTS})
    SET(in_file "${CMAKE_CURRENT_SOURCE_DIR}/${src}")
    GET_FILENAME_COMPONENT(WORKING_DIR ${in_file} PATH)
    STRING(REPLACE ".xml" ".c" src ${src})
    SET(out_file "${DIRECTORY}/${src}")
    GET_FILENAME_COMPONENT(OUPUT_DIR ${out_file} PATH)
    FILE(MAKE_DIRECTORY ${OUPUT_DIR})
    LIST(APPEND out_files "${DIRECTORY}/${src}")

    #FIXME implicit depends currently not working
    EXECUTE_PROCESS(
      COMMAND
        ${GLIB2_COMPILE_RESOURCES_EXECUTABLE}
          "--generate-dependencies"
          ${in_file}
      WORKING_DIRECTORY ${WORKING_DIR}
      OUTPUT_VARIABLE in_file_dep
    )
    STRING(REGEX REPLACE "(\r?\n)" ";" in_file_dep "${in_file_dep}")
    SET(in_file_dep_path "")
    FOREACH(dep ${in_file_dep})
      LIST(APPEND in_file_dep_path "${WORKING_DIR}/${dep}")
    ENDFOREACH(dep ${in_file_dep})
    ADD_CUSTOM_COMMAND(
      OUTPUT ${out_file}
      WORKING_DIRECTORY ${WORKING_DIR}
      COMMAND
        ${GLIB2_COMPILE_RESOURCES_EXECUTABLE}
      ARGS
        "--generate-source"
        "--target=${out_file}"
        ${in_file}
      DEPENDS
        ${in_file};${in_file_dep_path}
    )
  ENDFOREACH(src ${ARGS_SOURCES} ${ARGS_UNPARSED_ARGUMENTS})
  SET(${output} ${out_files} PARENT_SCOPE)
ENDFUNCTION(GLIB_COMPILE_RESOURCES)

FUNCTION(GLIB_COMPILE_RESOURCES_FULLPATH output)
  CMAKE_PARSE_ARGUMENTS(ARGS "" "SOURCE" ${ARGN})
  SET(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  SET(out_files "")

  FOREACH(src ${ARGS_SOURCE} ${ARGS_UNPARSED_ARGUMENTS})
    SET(in_file "${src}")
    GET_FILENAME_COMPONENT(WORKING_DIR ${in_file} PATH)
    STRING(REPLACE ".xml" ".c" src ${src})
    SET(out_file "${DIRECTORY}/${src}")
    GET_FILENAME_COMPONENT(OUPUT_DIR ${out_file} PATH)
    FILE(MAKE_DIRECTORY ${OUPUT_DIR})
    LIST(APPEND out_files "${DIRECTORY}/${src}")

    #FIXME implicit depends currently not working
    EXECUTE_PROCESS(
      COMMAND
          ${GLIB2_COMPILE_RESOURCES_EXECUTABLE}
          "--generate-dependencies"
          ${in_file}
      WORKING_DIRECTORY ${WORKING_DIR}
      OUTPUT_VARIABLE in_file_dep
    )
    STRING(REGEX REPLACE "(\r?\n)" ";" in_file_dep "${in_file_dep}")
    SET(in_file_dep_path "")
    FOREACH(dep ${in_file_dep})
      LIST(APPEND in_file_dep_path "${WORKING_DIR}/${dep}")
    ENDFOREACH(dep ${in_file_dep})
    ADD_CUSTOM_COMMAND(
      OUTPUT ${out_file}
      WORKING_DIRECTORY ${WORKING_DIR}
      COMMAND
        ${GLIB2_COMPILE_RESOURCES_EXECUTABLE}
      ARGS
        "--generate-source"
        "--target=${out_file}"
        ${in_file}
      DEPENDS
        ${in_file};${in_file_dep_path}
    )
  ENDFOREACH(src ${ARGS_SOURCES} ${ARGS_UNPARSED_ARGUMENTS})
  SET(${output} ${out_files} PARENT_SCOPE)
ENDFUNCTION(GLIB_COMPILE_RESOURCES_FULLPATH)

macro(add_test_executable EXE_NAME)
    add_test(${EXE_NAME} ${GLIB2_GTESTER_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME})
endmacro()
