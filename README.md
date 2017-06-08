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
* Install from AUR [appmenu-gtk-module-git](https://aur.archlinux.org/packages/appmenu-gtk-module-git/) for GTK applications to work
* Install [Appmenu](https://aur.archlinux.org/packages/appmenu-qt/) to get appmenu for Qt4 Applications to work. Qt 5.7 must work out of the box.
* Install these [libdbusmenu-glib](https://archlinux.org/packages/libdbusmenu-glib/) [libdbusmenu-gtk3](https://archlinux.org/packages/libdbusmenu-gtk3/) [libdbusmenu-gtk2](https://archlinux.org/packages/libdbusmenu-gtk2/) to get Chromium/Google Chrome to work

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
