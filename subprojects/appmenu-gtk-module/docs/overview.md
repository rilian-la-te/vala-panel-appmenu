---
title: Overview
origin: appmenu-gtk-parser/docs/overview.md
---

# appmenu-gtk-module parser

The **appmenu-gtk-parser** library exposes a GTK+ `GtkMenuShell` as a
[`Gio.MenuModel`](https://docs.gtk.org/gio/class.MenuModel.html) so it can be
exported over DBus (for example, by the global-menu module).

## Overview

- [`UnityGtkMenuShell`](class.MenuShell.html) is an opaque `GMenuModel` proxy that
  wraps a [`GtkMenuShell`](https://docs.gtk.org/gtk3/class.MenuShell.html) and
  presents its structure as a menu model.
- [`UnityGtkActionGroup`](class.ActionGroup.html) collects the actions backing a
  `UnityGtkMenuShell`, so that menu activations keep working through the model.

## Usage

Create a shell from an existing `GtkMenuShell`:

```c
UnityGtkMenuShell *shell = unity_gtk_menu_shell_new(gtk_menu_bar);
```

The resulting `shell` is a `GMenuModel` and can be handed to any API that consumes
menu models, such as the DBusMenu exporter in *appmenu-glib-translator*.
