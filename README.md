Vala Panel Extras
---

This is StatusNotifierItems for using in Vala Panel (but can be used any DE in list below). Supported DE is:
 * XFCE (via xfce4-sntray-plugin or xfce4-snw-plugin)
 * ValaPanel (SNTray)
 * KDE (native)
 * Gnome (via gnome-shell-extension-appindicator)
 * Unity (native)
 * Enlightenment (native)

*Done:*
 * ALSA Volume Applet (will fully work only in Vala Panel or xfce4-sntray-plugin)
 * XKB
 * Weather
 * UPower (battery status)

*TODO:*
 * Brightness applet (will fully work only in Vala Panel or xfce4-sntray-plugin)
 * Network status (maybe Connman applet)
 * CPU temperature
 * CPU frequrency

*Dependencies:*

*All:*
 * GLib (>= 2.40.0)
 * GTK+ (>= 3.12.0)
 * valac (>= 0.24.0)

*Weather:*
 * libgweather (>= 3.12.0)
*ALSA:*
 * libasound 
*XKB:*
 * libxkbcommon (>=0.5.0)
 * xkb-xcb
 * setxkbmap (temporary)
 * gdk-x11-3.0 (>= 3.12.0)

Author
===
 * Athor <ria.freelander@gmail.com>

Special thanks:
===
 * Ikey Doherty <ikey@evolve-os.com> for sidebar widget and icontasklist
