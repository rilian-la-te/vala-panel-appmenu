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
 
---
Usage Instructions
---
**XFCE**
- Type following into your console:
`xfconf-query -c xsettings -p /Gtk/Modules -n -t string -s "appmenu-gtk-module"`

**BUDGIE***
- Type following into your console:
`gsettings set org.gnome.settings-daemon.plugins.xsettings enabled-gtk-modules "['appmenu-gtk-module']"`

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


