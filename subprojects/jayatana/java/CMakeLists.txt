include(UseJava)
configure_file(core/basic/NativeLibraries.java.in ${CMAKE_CURRENT_BINARY_DIR}/core/basic/NativeLibraries.java)
configure_file(agent/Agent.java.in ${CMAKE_CURRENT_BINARY_DIR}/agent/Agent.java)
set(JAVA_SOURCES
    core/Feature.java
    core/FeatureManager.java
    core/FeatureWrapper.java
    core/basic/GMainLoop.java
    core/basic/GlobalMenuAdapter.java
    core/basic/GlobalMenu.java
    ${CMAKE_CURRENT_BINARY_DIR}/core/basic/NativeLibraries.java
    core/swing/SwingGlobalMenu.java
    core/swing/SwingGTKFixed.java
    core/swing/SwingWMClass.java
    core/swing/SwingGlobalMenuWindow.java
    ${CMAKE_CURRENT_BINARY_DIR}/agent/Agent.java
    )
add_jar(jayatana-java
    SOURCES
        core/Feature.java
        core/FeatureManager.java
        core/FeatureWrapper.java
        core/basic/GMainLoop.java
        core/basic/GlobalMenuAdapter.java
        core/basic/GlobalMenu.java
        ${CMAKE_CURRENT_BINARY_DIR}/core/basic/NativeLibraries.java
        core/swing/SwingGlobalMenu.java
        core/swing/SwingGTKFixed.java
        core/swing/SwingWMClass.java
        core/swing/SwingGlobalMenuWindow.java
    OUTPUT_NAME
        jayatana
    GENERATE_NATIVE_HEADERS
        java-headers
    DESTINATION
        ${CMAKE_CURRENT_BINARY_DIR}/../lib/
    )
if (${CMAKE_VERSION} VERSION_LESS 3.11.0)
create_javah(TARGET java-headers
    CLASSES
        com.jarego.jayatana.basic.GMainLoop
        com.jarego.jayatana.basic.GlobalMenu
    CLASSPATH
        jayatana-java
    )
endif()
install_jar(jayatana-java DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/java)
configure_file(agent/MANIFEST.MF.in ${CMAKE_CURRENT_BINARY_DIR}/agent/MANIFEST.MF)
add_jar(agent-java
    SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/agent/Agent.java
    OUTPUT_NAME
        jayatanaag
    MANIFEST
        ${CMAKE_CURRENT_BINARY_DIR}/agent/MANIFEST.MF
    )
install_jar(agent-java DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/java)
add_custom_target(java-files-ide SOURCES ${JAVA_SOURCES} agent/Agent.java.in core/basic/NativeLibraries.java.in)
