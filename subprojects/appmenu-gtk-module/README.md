---
Application Menu GTK+ Module
---

This is renamed port of [Launchpad repository](https://launchpad.net/unity-gtk-module) of Unity GTK+ Module.

Unity GTK+ Module is small GTK Module than strips menus from all GTK programs, converts to MenuModel and send to AppMenu.
Ubuntu users now does not need to install this.

**REQUIRED DEPENDENCES**

*All:*
 * GLib (>= 2.50.0)
 * GTK+ (>= 3.22.0)

*GTK2 Support*
 * GTK+ (>= 2.24.0)

*Demos*
 * valac (>= 0.24.0)

*Wayland support*
 * appmenu-glib-translator (>= 26.11)
 * libwayland-client
 * wayland-scanner

---
Backends
---
This module now provides 3 backends. X11 - working as before. Gtk-Shell - usable only for Gnome-Shell applets or on Wayfire.
Kwin-wayland - usable on KDE and Wayfire. When dbus_annotations will be merged, there would be third backend for Wayland.
Backend requests are welcome - module is now modular. And yes - it capable to export GMenuModel as DBusMenu now, thanks glib-translator.

---
Usage Instructions
---
**XFCE**
- Type following into your console:
`xfconf-query -c xsettings -p /Gtk/Modules -n -t string -s "appmenu-gtk-module"`

**BUDGIE***
- Type following into your console:
`gsettings set org.gnome.settings-daemon.plugins.xsettings enabled-gtk-modules "['appmenu-gtk-module']"`

**SYSTEMD SERVICE**
- Type following into your console:
`systemctl --user enable --now appmenu-gtk-module.service`

**OTHER**
- Create file .gtkrc-2.0 into your home directory, if it do not exists already
    - Add to this file `gtk-modules="appmenu-gtk-module"`
- Create file .config/gtk-3.0/settings.ini into your home directory, if it do not exists already
Add to this file:
    - If it is just created, `[Settings]`
    - And then ``gtk-modules="appmenu-gtk-module"``

**IF ABOVE DOES NOT WORK**
* Add to .profile or .bashrc:

```sh
if [ -n "$GTK_MODULES" ]; then
    GTK_MODULES="${GTK_MODULES}:appmenu-gtk-module"
else
    GTK_MODULES="appmenu-gtk-module"
fi

if [ -z "$UBUNTU_MENUPROXY" ]; then
    UBUNTU_MENUPROXY=1
fi

export GTK_MODULES
export UBUNTU_MENUPROXY
```

**IF NONE OF THESE ARE WORKING**
* Add above snippet to any place where environment variables should set.

**YOU SHOULD RELOGIN AFTER INSTALLING THIS MODULE FIRST TIME**


