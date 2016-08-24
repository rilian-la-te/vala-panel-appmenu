Development is suspended for year (I am in the army now)
---
Vala Panel Application Menu
---

This is Global Menu for using in Vala Panel and xfce4-panel. unity-gtk-module is used as backend.

*Dependencies:*

*All:*
 * GLib (>= 2.40.0)
 * GTK+ (>= 3.12.0)
 * valac (>= 0.24.0)
 * libbamf (>=0.5.0)

---
Installation
===

*Create a .gtkrc in your home(~) directory

  `export UBUNTU_MENUPROXY=libappmenu.so`
`export   GTK_MODULES=unity-gtk-module:$GTK_MODULES`


Arch Linux
===
* Install from AUR [unity-gtk-module-standalone-bzr](https://aur.archlinux.org/packages/unity-gtk-module-standalone-bzr/) for appmenu to work
* Install both [Appmenu](https://aur.archlinux.org/packages/appmenu-qt/) & [Appmenu-qt5](https://aur.archlinux.org/packages/appmenu-qt5-bzr/) to get appmenu for Qt Applications to work

Author
===
 * Author <ria.freelander@gmail.com>

Special thanks:
===
 * Ikey Doherty <ikey@evolve-os.com> for sidebar widget and icontasklist
