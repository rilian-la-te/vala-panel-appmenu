---
Vala Panel Application Menu
---

Vala Panel Application Menu is a Global Menu applet for use with Vala Panel, xfce4-panel and mate-panel (Budgie 10.x is also planned). unity-gtk-module is used as a backend, and thus must also be installed (see instructions, below).

**REQUIRED DEPENDENCES**

 * GLib (>= 2.50.0)
 * GTK+ (>= 3.22.0)
 * valac (>= 0.24.0)
 * libbamf (>=0.5.0)

---
Compilation Instructions (Non-Distribution-Specific)
---
  * Install all the required dependencies listed above. These packages may have different names depending on your distribution. Please see [below](#dependency-packages) for the package names on some specific distributions (i.e. Ubuntu).
  * Clone this repository to your `home` directory by typing:
  `git clone https://gitlab.com/vala-panel-project/vala-panel-appmenu.git` then `cd` into the directory.
  * You're **almost** ready to run `meson`. First, review the following flags:
    * Meson flags:
      * `-Dxfce=[enabled/disabled]` Use `enabled` to compile applet for XFCE Panel
      * `-Dbudgie=[enabled/disabled]` Use `enabled` to compile for budgie (experimental)
      * `-Dvalapanel=[enabled/disabled]` Use `enabled` to compile for Vala Panel
      * `-Dmate=[enabled/disabled]` Use `enabled` to compile for MATE Panel
      * `-Djayatana=[enabled/disabled]` Use `enabled` to include Jayatana library (enable global menu for java swing applications), requires CMake
      * `-Dappmenu-gtk-module=enabled` Use this flag if you are compiling for a distro other than Arch (see instructions below for including unity-gtk-module with Arch) or Ubuntu (Ubuntu users can install unity-gtk-module from the ubuntu repositories--see 'Post-build Instructions', below).
      * `--prefix=[path]` By default, Vala-Panel-Appmenu will install in the `/usr/local` directory. You can use this flag to change that. For some DEs (XFCE, for example), it is required to match install prefix with panel prefix (`/usr` in most distros), so, do not forget it.
  * once you've decided on any flags you want to include, type (from your build directory) `meson [flags] [dir]`
  * once the build is successful, you can compile and install Vala-Panel-Appmenu by typing `cd [dir] && ninja && sudo ninja install`
---
Post-Build Instructions
---
- Install bamfdaemon (if it is not bundled with libbamf)
  - It is strongly recommend to add bamfdaemon to autostart
- Install GTK module using instructions below
- To get QT menus to work, install your distribution's qt4 and qt5 appmenu packages. In Ubuntu 17.04, for example, this involves typing `sudo apt-get install appmenu-qt`
  
To install and enable unity-gtk-module for your distro:

 **UBUNTU-BASED DISTROS**
 - Install unity-gtk-module by typing `sudo apt-get install unity-gtk-module-common unity-gtk2-module unity-gtk3-module`
 - Follow instructions in (appmenu-gtk-module) [README](subprojects/appmenu-gtk-module/README.md), but replace any occurence of `appmenu-gtk-module` to `unity-gtk-module`

 **ARCH-BASED DISTROS**
* Install from AUR [appmenu-gtk-module-git](https://aur.archlinux.org/packages/appmenu-gtk-module-git/) for GTK applications to work
* Install [Appmenu](https://www.archlinux.org/packages/community/x86_64/appmenu-qt4/) to get appmenu for Qt4 Applications to work. Qt 5.7 must work out of the box.
* Install these [libdbusmenu-glib](https://archlinux.org/packages/libdbusmenu-glib/) [libdbusmenu-gtk3](https://archlinux.org/packages/libdbusmenu-gtk3/) [libdbusmenu-gtk2](https://archlinux.org/packages/libdbusmenu-gtk2/) to get Chromium/Google Chrome to work
 - Follow instructions in the (appmenu-gtk-module) [README](subprojects/appmenu-gtk-module/README.md), if it is not enabled automatically.

 **DISTROS OTHER THAN ARCH OR UBUNTU**
 - When building vala-panel-appmenu with meson, use the flag, `-Dappmenu-gtk-module=enabled`
 - Follow instructions in the (appmenu-gtk-module) [README](subprojects/appmenu-gtk-module/README.md)


**NOTE**: 
Vala-Panel-Appmenu conflicts with [qt5ct](https://sourceforge.net/p/qt5ct/tickets/34/) before 21.04.2017, so, if you are using an older version of qt5ct, use another PlatformTheme.

---
Desktop Environment-Specific Settings
---
When using the Vala-panel-appmenu as an XFCE or MATE menu applet, you have to configure the appmenu to show in the panel applet, rather than on each individual window. This configuration should remove any 'double' menus you may experience:

**XFCE**
- If you are using Vala-Panel-Appmenu for XFCE-Panel, type the following lines into your console:
```
xfconf-query -c xsettings -p /Gtk/ShellShowsMenubar -n -t bool -s true
xfconf-query -c xsettings -p /Gtk/ShellShowsAppmenu -n -t bool -s true
```

**MATE**
- Enable the appmenu and menubar in gtk with these steps:
- If you are using MATE>=1.19 (or 1.18 in Ubuntu), use this commands:
```
gsettings set org.mate.interface gtk-shell-shows-app-menu true
gsettings set org.mate.interface gtk-shell-shows-menubar true
```

**BUDGIE**
- If you using gnome-settings-daemon, you should go to dconf-editor and set key `org.gnome.settings-daemon.plugins.xsettings.overrides` to `{'Gtk/ShellShowsAppMenu': <0>, 'Gtk/ShellShowsMenubar': <1>}`

- If commands above does not work, create or edit .config/gtk-3.0/settings.ini file in your home(~) directory and add the following lines to it under `[Settings]`:
```
  gtk-shell-shows-app-menu=true
  gtk-shell-shows-menubar=true
```

---
Experimental Features
---
**JAyatana**

JAyatana allows for displaying global menus in Java Swing applications. Because Vala-Panel-Appmenu uses the unity-gtk-module backend, this should theoretically work with JAyatana, although applications such as Netbeans and the JetBrains suite of IDEs require some configuration, which you can figure out with a cursory internet search.

There are some problems with the implementation, notably that you need to include `env XDG_CURRENT_DESKTOP=Unity` to the beginning of your launch command.

Basic Instructions for Enabling JAyatana:
* Install OpenJDK >= 9 or JDK >= 1.9
* Build vala-panel-appmenu with `-DENABLE_JAYATANA=ON`
* Add following lines to your ~/.profile and ~/.bashrc, in any order:
```
export _JAVA_OPTIONS="${_JAVA_OPTIONS} -javaagent:/usr/share/java/jayatanaag.jar"
export JAYATANA_FORCE=1
```

---
Dependency Packages
---
**Ubuntu 18.04**

*All variants require:* `git build-essential cmake bamfdaemon libdbusmenu-glib-dev libgtk2.0-dev gobject-introspection libbamf3-dev libdbus-glib-1-dev libffi-dev libgirepository1.0-dev libpeas-dev libstartup-notification0-dev libwnck-3-dev libwnck-common libwnck22 libxml2-utils libxres-dev libvala-0.40-0 valac valac-0.40-vapi`

*Ubuntu Budgie also needs:* `budgie-core-dev gir1.2-budgie-1.0 libbudgie-plugin0 libbudgietheme0 libraven0`

*Ubuntu MATE also needs:* `libmate-panel-applet-dev`

*Xubuntu also needs:* `exo-utils libexo-1-0 libexo-2-0 libexo-common libexo-helpers libgarcon-1-0 libgarcon-common libxfce4panel-2.0-4 libxfce4panel-2.0-dev libxfce4ui-1-0 libxfce4ui-2-0 libxfce4ui-common libxfce4util7 libxfce4util-common libxfce4util-dev libxfconf-0-2 libxfconf-0-dev xfce4-panel xfce4-panel-dev xfconf`

*Vala Panel also needs:* `libvalapanel0 libvalapanel-dev`

Author
===
 * Athor <ria.freelander@gmail.com>
