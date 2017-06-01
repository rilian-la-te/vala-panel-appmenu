include(UseJava)
configure_file(core/basic/NativeLibraries.java.in ${CMAKE_CURRENT_BINARY_DIR}/core/basic/NativeLibraries.java)
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
    agent/Agent.java
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
    )
create_javah(TARGET java-headers
    CLASSES
        com.jarego.jayatana.basic.GMainLoop
        com.jarego.jayatana.basic.GlobalMenu      
    CLASSPATH
        jayatana-java
    )
install_jar(jayatana-java DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/java)
add_jar(agent-java
    SOURCES
        agent/Agent.java
    OUTPUT_NAME
        jayatanaag
    )
install_jar(agent-java DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/java)
add_custom_target(java-files-ide SOURCES ${JAVA_SOURCES})
