This is renamed fork of [Google Code repo](https://code.google.com/archive/p/java-swing-ayatana) of Jayatana.

JAyatana helps to use DBusMenu for Java Swing programs (enables Global Menu for many Java applications).

*Dependencies:*

 * GLib (>= 2.40.0)
 * Java (>= 1.9.0)
 * xkbcommon (>= 0.5.0)
 * dbusmenu-glib (>= 16.04.0)

*Enabling Jayatana*

1. System-wide way (recommended only if you have many Java programs with menus):
- add `JAVA_TOOL_OPTIONS=-javaagent:/usr/share/java/jayatanaag.jar $JAVA_TOOL_OPTIONS` into your *.profile* file.

2. Application-specific ways (useful if you usually have one or 2 Java programs, like Android Studio) and if above does not work.

*2.1. Intellij programs (Idea, PhpStorm, CLion, Android Studio)*
- Edit *.vmoptions file, and add `-javaagent:/usr/share/java/jayatanaag.jar` to the end of file.
- Edit *.properties file, and add `linux.native.menu=true` to the end of it.

*2.2. Netbeans*
- Edit netbeans.conf, and add `-J-javaagent:/usr/share/java/jayatanaag.jar` to the end of it.

3. Enable agent via desktop file (for any single application)
- Add `-javaagent:/usr/share/java/jayatanaag.jar` after Exec or TryExec line of application's desktop file (if application executes JAR directly). If application executes JAR via wrapper, and this option to the end of JVM options for running actual JAR.
