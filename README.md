---
Vala Panel Application Menu
---

This is Global Menu for using in Vala Panel, xfce4-panel and mate-panel (Budgie 10.x also planned). unity-gtk-module is used as backend.

*Dependencies:*

*All:*
 * GLib (>= 2.40.0)
 * GTK+ (>= 3.12.0)
 * valac (>= 0.24.0)
 * libbamf (>=0.5.0)
 
 ** Note: ensure that you install the respective 'dev' components of the aforementioned dependencies. Some distros (such as    
    Ubuntu) may split these up into separate packages. **
    
---
*Instructions for Compiling*
---
  Once you've cloned this repository, `cd` into the directory, then:
  * type `git submodule init && git update` to download the submodules (this will download the cmake and dbus-menu submodules you see in the repository, above)
  * type `mkdir build && cd build` 
  * You're now ready to run `cmake`. Review the following flags:
    * CMAKE FLAGS:
      -- `-DENABLE_XFCE=[ON/OFF]` Use `ON` to compile applet for XFCE Panel
      -- `-DENABLE_BUDGIE=[ON/OFF]` Use `ON` to compile for budgie (experimental)
      -- `-DENABLE_VALAPANEL=[ON/OFF]` Use `ON` to compile for Vala Panel
      -- `-DENABLE_MATE=[ON/OFF]` Use `ON` to compile for MATE Panel
      -- `-DENABLE_JAYATANA=[ON/OFF]` Use `ON` to include Jayatana library (enable global menu for java swing applications)
      -- `-DENABLE_UNITY_GTK_MODULE=ON` Use this flag if you are compiling for a distro other than Arch (see instructions below for including unity-gtk-module with Arch) or Ubuntu (Ubuntu users can install unity-gtk-module from the ubuntu repositories using `sudo apt-get install unity-gtk-module-common`).
      -- `-DCMAKE_INSTALL_PREFIX=[path]` By default, Vala-Panel-Appmenu will install in the `/usr/local` directory. You can use this flag to change that.
  * once you've decided on any flags you want to include, type (from the build directory) `cmake [flags] ..`
  * once the build is successful, you can compile and install Vala-Panel-Appmenu by typing make && sudo make install
---
*Installation after building*
---
* Create a .gtkrc in your home(~) directory
* Create a .config/gtk-3.0/settings.ini file in your home(~) directory
* Add to .config/gtk-3.0/settings.ini file in all distros:
`gtk-shell-shows-app-menu=true`
`gtk-shell-shows-menubar=true`
* It conflicts with [qt5ct](https://sourceforge.net/p/qt5ct/tickets/34/) before 21.04.2017, so, if you using old qt5ct, use another PlatformTheme
* Add to .gtkrc-2.0 file in Ubuntu-based:
`gtk-shell-shows-app-menu=1`
`gtk-shell-shows-menubar=1`
* For XFCE Only: 
`xfconf-query -c xsettings -p /Gtk/ShellShowsMenubar -n -t bool -s true`
`xfconf-query -c xsettings -p /Gtk/ShellShowsAppmenu -n -t bool -s true`
* Add to .profile and .bashrc on Ubuntu:
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

*Arch Linux*
* Install from AUR [unity-gtk-module-standalone-bzr](https://aur.archlinux.org/packages/unity-gtk-module-standalone-bzr/) for appmenu to work
* Install [Appmenu](https://aur.archlinux.org/packages/appmenu-qt/) to get appmenu for Qt4 Applications to work. Qt 5.7 must work out of the box.
* Install these [libdbusmenu-glib](https://aur.archlinux.org/packages/libdbusmenu-glib/) [libdbusmenu-gtk3](https://aur.archlinux.org/packages/libdbusmenu-gtk3/) [libdbusmenu-gtk2](https://aur.archlinux.org/packages/libdbusmenu-gtk2/) to get Chromium/Google Chrome to work

*For distros other than Arch or Ubuntu*
* Build vala-panel-appmenu with `-DENABLE_UNITY_GTK_MODULE=ON`
* Follow instructions into [README](unity-gtk-module/README.md)

*For JAyatana*
* Install OpenJDK >= 7 or JDK >= 1.7
* Build vala-panel-appmenu with `-DENABLE_JAYATANA=ON`
* Find instructions in Internet and made a Pull Request:)))

Author
===
 * Athor <ria.freelander@gmail.com>
