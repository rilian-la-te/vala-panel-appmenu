---
Vala Panel Application Menu
---

Vala Panel Application Menu is a Global Menu applet for use with Vala Panel, xfce4-panel and mate-panel (Budgie 10.x is also planned). unity-gtk-module is used as a backend, and thus must also be installed (see instructions, below).

**REQUIRED DEPENDENCES**

 * GLib (>= 2.40.0)
 * GTK+ (>= 3.12.0)
 * valac (>= 0.24.0)
 * libbamf (>=0.5.0)
    
---
Compilation Instructions
---
  * Clone this repository to your `home` directory by typing:
  `git clone https://github.com/rilian-la-te/vala-panel-appmenu.git` then `cd` into the directory.
  * type `git submodule init && git submodule update` to download the submodules (this will download the cmake and dbus-menu submodules you see in the repository, above)
  * type `mkdir build && cd build` (to keep things tidy)
  * You're **almost** ready to run `cmake`. First, review the following flags:
    * CMAKE FLAGS:
      * `-DENABLE_XFCE=[ON/OFF]` Use `ON` to compile applet for XFCE Panel
      * `-DENABLE_BUDGIE=[ON/OFF]` Use `ON` to compile for budgie (experimental)
      * `-DENABLE_VALAPANEL=[ON/OFF]` Use `ON` to compile for Vala Panel
      * `-DENABLE_MATE=[ON/OFF]` Use `ON` to compile for MATE Panel
      * `-DENABLE_JAYATANA=[ON/OFF]` Use `ON` to include Jayatana library (enable global menu for java swing applications)
      * `-DENABLE_UNITY_GTK_MODULE=ON` Use this flag if you are compiling for a distro other than Arch (see instructions below for including unity-gtk-module with Arch) or Ubuntu (Ubuntu users can install unity-gtk-module from the ubuntu repositories--see 'Post-build Instructions', below).
      * `-DMAKE_BOLD_APPNAME=[ON/OFF]` Use `ON` to make appname menu font bold (similar to macOS menu style).
      * `-DCMAKE_INSTALL_PREFIX=[path]` By default, Vala-Panel-Appmenu will install in the `/usr/local` directory. You can use this flag to change that.
  * once you've decided on any flags you want to include, type (from your build directory) `cmake [flags] ..`
  * once the build is successful, you can compile and install Vala-Panel-Appmenu by typing `make && sudo make install`
---
Post-Build Instructions
---
- Enable unity-gtk-module on Ubuntu:
  - Create a .gtkrc in your home(~) directory, if there isn't one already.
  - Add the following lines to this .gtkrc file:
   `gtk-shell-shows-app-menu=1`
   `gtk-shell-shows-menubar=1`
  - Add the following lines to your .profile and .bashrc (in your home directory) in order to enable the unity-gtk-module:
    ```sh
    if [ -n "$GTK_MODULES" ]; then
        GTK_MODULES="${GTK_MODULES}:unity-gtk-module"
    else
        GTK_MODULES="unity-gtk-module"
    fi
 
    if [ -z "$UBUNTU_MENUPROXY" ]; then
        UBUNTU_MENUPROXY=1
    fi

    export GTK_MODULES
    export UBUNTU_MENUPROXY
    ```
  
To install unity-gtk-module for your distro:

 **UBUNTU-BASED DISTROS**
 - Install unity-gtk-module by typing `sudo apt-get install unity-gtk-module-common unity-gtk2-module unity-gtk3-module`

 **ARCH-BASED DISTROS**
* Install from AUR [appmenu-gtk-module-git](https://aur.archlinux.org/packages/appmenu-gtk-module-git/) for GTK applications to work
* Install [Appmenu](https://aur.archlinux.org/packages/appmenu-qt/) to get appmenu for Qt4 Applications to work. Qt 5.7 must work out of the box.
* Install these [libdbusmenu-glib](https://archlinux.org/packages/libdbusmenu-glib/) [libdbusmenu-gtk3](https://archlinux.org/packages/libdbusmenu-gtk3/) [libdbusmenu-gtk2](https://archlinux.org/packages/libdbusmenu-gtk2/) to get Chromium/Google Chrome to work

 **DISTROS OTHER THAN ARCH OR UBUNTU**
 - When building vala-panel-appmenu with CMAKE, use the flag, `-DENABLE_UNITY_GTK_MODULE=ON`
 - Follow instructions in the (unity-gtk-module) [README](unity-gtk-module/README.md)

**ALL DISTROS**
- To get QT menus to work, install your distribution's qt4 and qt5 appmenu packages. In Ubuntu 17.04, for example, this involves typing `sudo apt-get install appmenu-qt`.

**NOTE**: 
Vala-Panel-Appmenu conflicts with [qt5ct](https://sourceforge.net/p/qt5ct/tickets/34/) before 21.04.2017, so, if you are using an older version of qt5ct, use another PlatformTheme.

---
Desktop Environment-Specific Settings
---
When using the Vala-panel-appmenu as an XFCE or MATE menu applet, you have to configure the appmenu to show in the panel applet, rather than on each individual window. This configuration should remove any 'double' menus you may experience:

**XFCE**
- If you are using Vala-Panel-Appmenu for XFCE-Panel, type the following lines into your console:
`xfconf-query -c xsettings -p /Gtk/ShellShowsMenubar -n -t bool -s true`
`xfconf-query -c xsettings -p /Gtk/ShellShowsAppmenu -n -t bool -s true`

**MATE**
- Enable the appmenu and menubar in gtk with these steps:
- If you are using MATE>=1.19 (or 1.18 in Ubuntu), use this commands:
`gsettings set org.mate.interface gtk-shell-shows-app-menu true`
`gsettings set org.mate.interface gtk-shell-shows-menubar true`
- If commands above does not work, create or edit .config/gtk-3.0/settings.ini file in your home(~) directory and add the following lines to it under `[Settings]`:
  `gtk-shell-shows-app-menu=true`
  `gtk-shell-shows-menubar=true`  

---
Experimental Features
---
**JAyatana**

JAyatana allows for displaying global menus in Java Swing applications. Because Vala-Panel-Appmenu uses the unity-gtk-module backend, this should theoretically work with JAyatana, although applications such as Netbeans and the JetBrains suite of IDEs require some configuration, which you can figure out with a cursory internet search.

There are some problems with the implementation, notably that you need to include `env XDG_CURRENT_DESKTOP=Unity` to the beginning of your launch command.

Basic Instructions for Enabling JAyatana:
* Install OpenJDK >= 7 or JDK >= 1.7
* Build vala-panel-appmenu with `-DENABLE_JAYATANA=ON`
* Find instructions on Internet

Author
===
 * Athor <ria.freelander@gmail.com>
