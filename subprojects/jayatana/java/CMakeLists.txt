include(UseJava)
configure_file(com/jarego/jayatana/basic/NativeLibraries.java.in ${CMAKE_CURRENT_BINARY_DIR}/com/jarego/jayatana/basic/NativeLibraries.java)
configure_file(com/jarego/jayatana/Agent.java.in ${CMAKE_CURRENT_BINARY_DIR}/agent/Agent.java)
set(CMAKE_JAVA_COMPILE_FLAGS --add-exports java.desktop/com.sun.java.swing.plaf.gtk=ALL-UNNAMED -Xlint:deprecation)
set(JAVA_SOURCES
    com/jarego/jayatana/Feature.java
    com/jarego/jayatana/FeatureManager.java
    com/jarego/jayatana/FeatureWrapper.java
    com/jarego/jayatana/basic/GMainLoop.java
    com/jarego/jayatana/basic/GlobalMenuAdapter.java
    com/jarego/jayatana/basic/GlobalMenu.java
    ${CMAKE_CURRENT_BINARY_DIR}/com/jarego/jayatana/basic/NativeLibraries.java
    com/jarego/jayatana/swing/SwingGlobalMenu.java
    com/jarego/jayatana/swing/SwingGTKFixed.java
    com/jarego/jayatana/swing/SwingWMClass.java
    com/jarego/jayatana/swing/SwingGlobalMenuWindow.java
    ${CMAKE_CURRENT_BINARY_DIR}/agent/Agent.java
    )
add_jar(jayatana-java
    SOURCES
        com/jarego/jayatana/Feature.java
        com/jarego/jayatana/FeatureManager.java
        com/jarego/jayatana/FeatureWrapper.java
        com/jarego/jayatana/basic/GMainLoop.java
        com/jarego/jayatana/basic/GlobalMenuAdapter.java
        com/jarego/jayatana/basic/GlobalMenu.java
        ${CMAKE_CURRENT_BINARY_DIR}/com/jarego/jayatana/basic/NativeLibraries.java
        com/jarego/jayatana/swing/SwingGlobalMenu.java
        com/jarego/jayatana/swing/SwingGTKFixed.java
        com/jarego/jayatana/swing/SwingWMClass.java
        com/jarego/jayatana/swing/SwingGlobalMenuWindow.java
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
add_custom_target(java-files-ide SOURCES ${JAVA_SOURCES} agent/Agent.java.in com/jarego/jayatana/basic/NativeLibraries.java.in)
