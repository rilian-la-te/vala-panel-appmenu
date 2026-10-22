# ✨🧉 Improved mate-appmenu

This version of [vala-panel-appmenu](https://gitlab.com/vala-panel-project/vala-panel-appmenu) is forked from the main project on GitLab. This improved version includes the following features focusing on global menu applet on MATE:

 - 🖼️ Preference to show the window icon
 - 🔍 **Replacement of mate-hud** 

`mate-hud` subproject **replaces legacy python mate-hud with a search option** delivered by this applet just like mate-panel run dialog. 

⚠️ **IMPORTANT:** The following instructions for building and installing this fork of Vala Panel Application Menu are for **ONLY ON DEBIAN and FEDORA** systems. 🛑 **NOT TESTED ON UBUNTU YET!!!** If you wish to use precompiled `appmenu-registar` and `appmenu-gtk-module` packages from Debian repos, `subprojects` can be disabled for compiling.

## 📦 Prerequisites

### Required Dependencies for Debian Stable

#### **Debian** Systems 🍥

```bash
sudo apt-get install git build-essential meson cmake ninja-build \
  libgtk2.0-dev libglib2.0-dev libgtk-3-dev libdbusmenu-glib-dev valac libwnck-3-dev libjson-glib-dev\
  libmate-panel-applet-dev gobject-introspection libgirepository1.0-dev default-jdk
```


## 🛠️ Building from Source

### 1. 📥 Clone the Repository

```bash
git clone https://gitlab.com/KeremSoke/mate-appmenu.git
cd mate-appmenu
```

### 2. ⚙️ Configure with Meson
**For Debian with MATE** 🐧🧉

```bash
meson setup build \
  --prefix=/usr \
  -Dmate=enabled \
  -Dmate-hud=enabled \
  -Dvalapanel=disabled \
  -Dxfce=disabled \
  -Dbudgie=disabled \
  -Dregistrar=enabled \
  -Djayatana=enabled \
  -Dappmenu-gtk-module=enabled
```

**🚩 Important flags**

Other flags can actually be disabled except these:
- `--prefix=/usr` - Install to `/usr` instead of `/usr/local` (required for MATE to find the applet)
- `-Dmate=enabled` - Build the MATE panel applet
- `-Dregistrar=enabled` - Build the D-Bus registrar service (recommended for compatibility with older/legacy applications)
- `-Dappmenu-gtk-module=enabled` - Build the GTK module (required for Debian/most distros)


### 3. 🔨 Compile and Install

```bash
meson compile -C build
sudo meson install -C build
```

### 4. 🔄 Recompile GSettings Schemas

After installation, recompile the GSettings schemas:

```bash
sudo glib-compile-schemas /usr/share/glib-2.0/schemas/
```

## 🚀 Post-Installation Setup

### 🔌 Enable GTK Module

The applet requires a GTK module to intercept application menus.
- 📝 Create file `.gtkrc-2.0` into your home directory, if it do not exists already
    - Add to this file `gtk-modules="appmenu-gtk-module"`
- 📝 Create file `.config/gtk-3.0/settings.ini` into your home directory, if it do not exists already
Add to this file:
    - If it is just created, `[Settings]`
    - And then ``gtk-modules=appmenu-gtk-module``

**❌ IF ABOVE DOES NOT WORK**
* Add to `.profile` or `.bashrc`:

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


#### Debian Systems 🍥

If you built with `-Dappmenu-gtk-module=enabled` (recommended), enable it by adding to `~/.profile` or `~/.bashrc`:

```bash
export GTK_MODULES="appmenu-gtk-module"
```

### 🖥️ Configure MATE Settings

To make MATE show menus in the panel applet instead of application windows:

```bash
gsettings set org.mate.interface gtk-shell-shows-app-menu true
gsettings set org.mate.interface gtk-shell-shows-menubar true
```

Alternatively, create or edit `~/.config/gtk-3.0/settings.ini` and add:
```ini
[Settings]
gtk-shell-shows-app-menu=true
gtk-shell-shows-menubar=true
```

## ➕ Adding the Applet to MATE Panel

1. 🖱️ Right-click on the MATE panel
2. Select **Add to Panel**
3. 🔎 Search for **AppMenu** or **Application Menu**
4. Click **Add**

## ⚙️ Applet Configuration

Right-click on the AppMenu applet and select **Preferences** OR MATE Control Center > Global Menu to configure:

- 🤏 **Use Compact mode** - Combines all menus under a single application menu
- **🇧 Use bold application name** - Makes the application name bold
- 🖼️ **Show app icon** - Displays the current application's icon before the menu (e.g., Firefox icon when Firefox is active)
- 🎚️ Adjust the padding for the activity icon.
- Various *HUD* anchoring settings 

---
ℹ️ *This guide is for mate-appmenu version 26.10 and later.*
