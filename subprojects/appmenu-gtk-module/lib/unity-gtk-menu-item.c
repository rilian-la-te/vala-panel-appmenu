/*
 * Copyright 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Ryan Lortie <desrt@desrt.ca>
 *          William Hua <william.hua@canonical.com>
 */

#include "unity-gtk-action-group-private.h"
#include "unity-gtk-menu-item-private.h"
#include <string.h>

G_DEFINE_TYPE(UnityGtkMenuItem, unity_gtk_menu_item, G_TYPE_OBJECT);

typedef struct _UnityGtkSearch UnityGtkSearch;

struct _UnityGtkSearch
{
	GType type;
	guint index;
	GObject *object;
};

static void g_object_get_nth_object(GObject *object, gpointer data)
{
	UnityGtkSearch *search = data;

	g_return_if_fail(G_IS_OBJECT(object));

	if (search->object == NULL)
	{
		if (g_type_is_a(G_OBJECT_TYPE(object), search->type))
		{
			if (search->index == 0)
				search->object = object;
			else
				search->index--;
		}

		if (search->object == NULL && GTK_IS_CONTAINER(object))
			gtk_container_forall(GTK_CONTAINER(object),
			                     (GtkCallback)g_object_get_nth_object,
			                     data);
	}
}

GtkLabel *gtk_menu_item_get_nth_label(GtkMenuItem *menu_item, guint index)
{
	UnityGtkSearch search;

	g_return_val_if_fail(GTK_IS_MENU_ITEM(menu_item), NULL);

	search.type   = GTK_TYPE_LABEL;
	search.index  = index;
	search.object = NULL;

	g_object_get_nth_object(G_OBJECT(menu_item), &search);

	return search.object != NULL ? GTK_LABEL(search.object) : NULL;
}

const char *gtk_menu_item_get_nth_label_label(GtkMenuItem *menu_item, guint index)
{
	GtkLabel *label;
	const char *label_label;

	g_return_val_if_fail(GTK_IS_MENU_ITEM(menu_item), NULL);

	label       = gtk_menu_item_get_nth_label(menu_item, index);
	label_label = NULL;

	if (label != NULL)
		label_label = gtk_label_get_label(label);

	return label_label != NULL && label_label[0] != '\0' ? label_label : NULL;
}

static GtkImage *gtk_menu_item_get_nth_image(GtkMenuItem *menu_item, guint index)
{
	UnityGtkSearch search;

	g_return_val_if_fail(GTK_IS_MENU_ITEM(menu_item), NULL);

	search.type   = GTK_TYPE_IMAGE;
	search.index  = index;
	search.object = NULL;

	g_object_get_nth_object(G_OBJECT(menu_item), &search);

	return search.object != NULL ? GTK_IMAGE(search.object) : NULL;
}

static GIcon *gtk_image_get_icon(GtkImage *image)
{
	GIcon *icon = NULL;

	g_return_val_if_fail(GTK_IS_IMAGE(image), NULL);

	switch (gtk_image_get_storage_type(image))
	{
	case GTK_IMAGE_GICON:
	{
		gtk_image_get_gicon(image, &icon, NULL);

		if (icon != NULL)
			g_object_ref(icon);
	}

	break;

	case GTK_IMAGE_ICON_NAME:
	{
		const char *name = NULL;

		gtk_image_get_icon_name(image, &name, NULL);

		if (name != NULL)
			icon = G_ICON(g_themed_icon_new_with_default_fallbacks(name));
	}

	break;

	case GTK_IMAGE_PIXBUF:
	{
		GdkPixbuf *pixbuf = gtk_image_get_pixbuf(image);

		if (pixbuf != NULL)
			icon = G_ICON(g_object_ref(pixbuf));
	}

	break;

	case GTK_IMAGE_ANIMATION:
	{
		GdkPixbufAnimation *animation = gtk_image_get_animation(image);

		if (animation != NULL)
		{
			GdkPixbuf *pixbuf = gdk_pixbuf_animation_get_static_image(animation);

			if (pixbuf != NULL)
				icon = G_ICON(g_object_ref(pixbuf));
		}
	}

	break;

	case GTK_IMAGE_STOCK:
#if GTK_MAJOR_VERSION == 2
	{
		char *stock      = NULL;
		GtkIconSize size = GTK_ICON_SIZE_INVALID;

		gtk_image_get_stock(image, &stock, &size);

		if (stock != NULL)
		{
			GdkPixbuf *pixbuf =
			    gtk_widget_render_icon(GTK_WIDGET(image), stock, size, NULL);

			if (pixbuf != NULL)
				icon = G_ICON(pixbuf);
		}
	}
#elif GTK_MAJOR_VERSION == 3
		G_GNUC_BEGIN_IGNORE_DEPRECATIONS
		{
			GtkStyleContext *context = gtk_widget_get_style_context(GTK_WIDGET(image));

			if (context != NULL)
			{
				char *stock      = NULL;
				GtkIconSize size = GTK_ICON_SIZE_INVALID;

				gtk_image_get_stock(image, &stock, &size);

				if (stock != NULL)
				{
					GtkIconSet *set =
					    gtk_style_context_lookup_icon_set(context, stock);

					if (set != NULL)
					{
						GdkPixbuf *pixbuf =
						    gtk_icon_set_render_icon_pixbuf(set,
						                                    context,
						                                    size);

						if (pixbuf != NULL)
							icon = G_ICON(pixbuf);
					}
				}
			}
		}
		G_GNUC_END_IGNORE_DEPRECATIONS
#endif

	break;

	case GTK_IMAGE_ICON_SET:
#if GTK_MAJOR_VERSION == 2
	{
		GtkIconSet *set  = NULL;
		GtkIconSize size = GTK_ICON_SIZE_INVALID;

		gtk_image_get_icon_set(image, &set, &size);

		if (set != NULL)
		{
			GtkStyle *style            = gtk_widget_get_style(GTK_WIDGET(image));
			GtkTextDirection direction = gtk_widget_get_direction(GTK_WIDGET(image));
			GtkStateType state         = gtk_widget_get_state(GTK_WIDGET(image));
			GdkPixbuf *pixbuf          = gtk_icon_set_render_icon(set,
                                                                     style,
                                                                     direction,
                                                                     state,
                                                                     size,
                                                                     GTK_WIDGET(image),
                                                                     NULL);

			if (pixbuf != NULL)
				icon = G_ICON(pixbuf);
		}
	}
#elif GTK_MAJOR_VERSION == 3
		G_GNUC_BEGIN_IGNORE_DEPRECATIONS
		{
			GtkStyleContext *context = gtk_widget_get_style_context(GTK_WIDGET(image));

			if (context != NULL)
			{
				GtkIconSet *set  = NULL;
				GtkIconSize size = GTK_ICON_SIZE_INVALID;

				gtk_image_get_icon_set(image, &set, &size);

				if (set != NULL)
				{
					GdkPixbuf *pixbuf =
					    gtk_icon_set_render_icon_pixbuf(set, context, size);

					if (pixbuf != NULL)
						icon = G_ICON(pixbuf);
				}
			}
		}
		G_GNUC_END_IGNORE_DEPRECATIONS
#endif

	break;

#if GTK_MAJOR_VERSION == 2
	case GTK_IMAGE_IMAGE:
	{
		GdkImage *gdk_image = NULL;

		gtk_image_get_image(image, &gdk_image, NULL);

		if (gdk_image != NULL)
		{
			GdkColormap *colourmap = gtk_widget_get_colormap(GTK_WIDGET(image));
			GdkPixbuf *pixbuf      = gdk_pixbuf_get_from_image(NULL,
                                                                      gdk_image,
                                                                      colourmap,
                                                                      0,
                                                                      0,
                                                                      0,
                                                                      0,
                                                                      gdk_image->width,
                                                                      gdk_image->height);

			if (pixbuf != NULL)
				icon = G_ICON(pixbuf);
		}
	}

	break;

	case GTK_IMAGE_PIXMAP:
	{
		GdkPixmap *pixmap = NULL;

		gtk_image_get_pixmap(image, &pixmap, NULL);

		if (pixmap != NULL)
		{
			GdkPixbuf *pixbuf;
			GdkColormap *colourmap;
			gint width  = 0;
			gint height = 0;

			gdk_pixmap_get_size(pixmap, &width, &height);
			colourmap = gtk_widget_get_colormap(GTK_WIDGET(image));
			pixbuf    = gdk_pixbuf_get_from_drawable(NULL,
                                                              pixmap,
                                                              colourmap,
                                                              0,
                                                              0,
                                                              0,
                                                              0,
                                                              width,
                                                              height);

			if (pixbuf != NULL)
				icon = G_ICON(pixbuf);
		}
	}

	break;
#endif

	default:
		break;
	}

	return icon;
}

static void unity_gtk_menu_item_handle_item_notify(GObject *object, GParamSpec *pspec,
                                                   gpointer user_data)
{
	static const char *label_name;
	static const char *use_underline_name;

	UnityGtkMenuItem *item;
	UnityGtkMenuShell *parent_shell;
	GObject *menu_item;
	const char *name;

	g_return_if_fail(UNITY_GTK_IS_MENU_ITEM(user_data));

	item         = UNITY_GTK_MENU_ITEM(user_data);
	parent_shell = item->parent_shell;
	menu_item    = G_OBJECT(item->menu_item);

	g_return_if_fail(parent_shell != NULL);
	g_warn_if_fail(object == menu_item);

	if (G_UNLIKELY(label_name == NULL))
		label_name = g_intern_static_string("label");
	if (G_UNLIKELY(use_underline_name == NULL))
		use_underline_name = g_intern_static_string("use-underline");

	name = g_param_spec_get_name(pspec);

	if (name != label_name && name != use_underline_name)
		unity_gtk_menu_shell_handle_item_notify(parent_shell, item, name);
}

static void unity_gtk_menu_item_handle_label_notify(GObject *object, GParamSpec *pspec,
                                                    gpointer user_data)
{
	static const char *label_name;
	static const char *use_underline_name;

	UnityGtkMenuItem *item;
	UnityGtkMenuShell *parent_shell;
	const char *name;

	g_return_if_fail(UNITY_GTK_IS_MENU_ITEM(user_data));

	item         = UNITY_GTK_MENU_ITEM(user_data);
	parent_shell = item->parent_shell;

	g_return_if_fail(parent_shell != NULL);

	if (G_UNLIKELY(label_name == NULL))
		label_name = g_intern_static_string("label");
	if (G_UNLIKELY(use_underline_name == NULL))
		use_underline_name = g_intern_static_string("use-underline");

	name = g_param_spec_get_name(pspec);

	if (name == label_name || name == use_underline_name)
		unity_gtk_menu_shell_handle_item_notify(parent_shell, item, name);
}

static void unity_gtk_menu_item_disconnect_labels(UnityGtkMenuItem *item)
{
	g_return_if_fail(UNITY_GTK_IS_MENU_ITEM(item));

	if (item->second_label != NULL)
	{
		g_signal_handlers_disconnect_by_data(item->second_label, item);
		item->second_label = NULL;
	}

	if (item->first_label != NULL)
	{
		g_signal_handlers_disconnect_by_data(item->first_label, item);
		item->first_label = NULL;
	}
}

static gboolean unity_gtk_menu_item_connect_labels(UnityGtkMenuItem *item)
{
	GtkLabel *first_label  = NULL;
	GtkLabel *second_label = NULL;

	g_return_val_if_fail(UNITY_GTK_IS_MENU_ITEM(item), FALSE);

	if (item->menu_item != NULL)
	{
		/* ensure label is available */
		gtk_menu_item_get_label(item->menu_item);

		first_label  = gtk_menu_item_get_nth_label(item->menu_item, 0);
		second_label = gtk_menu_item_get_nth_label(item->menu_item, 1);
	}

	if (first_label != item->first_label || second_label != item->second_label)
	{
		unity_gtk_menu_item_disconnect_labels(item);

		item->first_label  = first_label;
		item->second_label = second_label;

		if (item->first_label != NULL)
			g_signal_connect(item->first_label,
			                 "notify",
			                 G_CALLBACK(unity_gtk_menu_item_handle_label_notify),
			                 item);
		if (item->second_label != NULL)
			g_signal_connect(item->second_label,
			                 "notify",
			                 G_CALLBACK(unity_gtk_menu_item_handle_label_notify),
			                 item);

		return TRUE;
	}

	return FALSE;
}

static void unity_gtk_menu_item_handle_add_or_remove(GtkContainer *container, GtkWidget *widget,
                                                     gpointer user_data)
{
	UnityGtkMenuItem *item;

	g_return_if_fail(UNITY_GTK_IS_MENU_ITEM(user_data));

	item = UNITY_GTK_MENU_ITEM(user_data);

	/* just ignore the case when parent_shell is NULL */
	if (item->parent_shell != NULL && unity_gtk_menu_item_connect_labels(item))
		unity_gtk_menu_shell_handle_item_notify(item->parent_shell, item, "label");
}

static void unity_gtk_menu_item_handle_accel_closures_changed(GtkWidget *widget, gpointer user_data)
{
	UnityGtkMenuItem *item;
	UnityGtkMenuShell *parent_shell;

	g_return_if_fail(UNITY_GTK_IS_MENU_ITEM(user_data));

	item         = UNITY_GTK_MENU_ITEM(user_data);
	parent_shell = item->parent_shell;

	g_return_if_fail(parent_shell != NULL);

	unity_gtk_menu_shell_handle_item_notify(parent_shell, item, "accel-path");
}

static void unity_gtk_menu_item_set_menu_item(UnityGtkMenuItem *item, GtkMenuItem *menu_item)
{
	g_return_if_fail(UNITY_GTK_IS_MENU_ITEM(item));

	if (menu_item != item->menu_item)
	{
		UnityGtkMenuShell *child_shell = item->child_shell;

		unity_gtk_menu_item_disconnect_labels(item);

		if (item->menu_item != NULL)
			g_signal_handlers_disconnect_by_data(item->menu_item, item);

		if (child_shell != NULL)
		{
			g_warn_if_fail(item->child_shell_valid);
			item->child_shell = NULL;
			g_object_unref(child_shell);
		}

		item->child_shell_valid = FALSE;
		item->menu_item         = menu_item;

		if (menu_item != NULL)
		{
			g_signal_connect(menu_item,
			                 "notify",
			                 G_CALLBACK(unity_gtk_menu_item_handle_item_notify),
			                 item);
			g_signal_connect(menu_item,
			                 "add",
			                 G_CALLBACK(unity_gtk_menu_item_handle_add_or_remove),
			                 item);
			g_signal_connect(menu_item,
			                 "remove",
			                 G_CALLBACK(unity_gtk_menu_item_handle_add_or_remove),
			                 item);
			g_signal_connect(menu_item,
			                 "accel-closures-changed",
			                 G_CALLBACK(
			                     unity_gtk_menu_item_handle_accel_closures_changed),
			                 item);

			/*
			 * LP: #1208019: We do this because Eclipse sets menu item
			 * accelerators using private API, and there's no way for us to
			 * detect when they change.
			 */
			if (gtk_menu_item_get_submenu(menu_item) != NULL)
				g_signal_emit_by_name(gtk_menu_item_get_submenu(menu_item), "show");
		}

		unity_gtk_menu_item_connect_labels(item);
	}
}

static void unity_gtk_menu_item_set_parent_shell(UnityGtkMenuItem *item,
                                                 UnityGtkMenuShell *parent_shell)
{
	g_return_if_fail(UNITY_GTK_IS_MENU_ITEM(item));

	item->parent_shell = parent_shell;
}

static void unity_gtk_menu_item_dispose(GObject *object)
{
	UnityGtkMenuItem *item;

	g_return_if_fail(UNITY_GTK_IS_MENU_ITEM(object));

	item = UNITY_GTK_MENU_ITEM(object);

	unity_gtk_menu_item_set_action(item, NULL);
	unity_gtk_menu_item_set_parent_shell(item, NULL);
	unity_gtk_menu_item_set_menu_item(item, NULL);

	G_OBJECT_CLASS(unity_gtk_menu_item_parent_class)->dispose(object);
}

static void unity_gtk_menu_item_finalize(GObject *object)
{
	UnityGtkMenuItem *item;

	g_return_if_fail(UNITY_GTK_IS_MENU_ITEM(object));

	item = UNITY_GTK_MENU_ITEM(object);

	g_free(item->label_label);
	item->label_label = NULL;

	G_OBJECT_CLASS(unity_gtk_menu_item_parent_class)->finalize(object);
}

static void unity_gtk_menu_item_class_init(UnityGtkMenuItemClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->dispose  = unity_gtk_menu_item_dispose;
	object_class->finalize = unity_gtk_menu_item_finalize;
}

static void unity_gtk_menu_item_init(UnityGtkMenuItem *self)
{
}

UnityGtkMenuItem *unity_gtk_menu_item_new(GtkMenuItem *menu_item, UnityGtkMenuShell *parent_shell,
                                          guint item_index)
{
	UnityGtkMenuItem *item = g_object_new(UNITY_GTK_TYPE_MENU_ITEM, NULL);

	unity_gtk_menu_item_set_menu_item(item, menu_item);
	unity_gtk_menu_item_set_parent_shell(item, parent_shell);
	item->item_index = item_index;

	return item;
}

UnityGtkMenuShell *unity_gtk_menu_item_get_child_shell(UnityGtkMenuItem *item)
{
	g_return_val_if_fail(UNITY_GTK_IS_MENU_ITEM(item), NULL);

	if (!item->child_shell_valid)
	{
		GtkMenuItem *menu_item         = item->menu_item;
		UnityGtkMenuShell *child_shell = item->child_shell;

		if (child_shell != NULL)
		{
			g_warn_if_reached();
			item->child_shell = NULL;
			g_object_unref(child_shell);
		}

		if (menu_item != NULL)
		{
			GtkWidget *submenu = gtk_menu_item_get_submenu(menu_item);

			if (submenu != NULL)
				item->child_shell =
				    unity_gtk_menu_shell_new_internal(GTK_MENU_SHELL(submenu));
		}

		item->child_shell_valid = TRUE;

		if (unity_gtk_menu_item_is_visible(item) && item->child_shell != NULL)
		{
			UnityGtkMenuShell *parent_shell = item->parent_shell;

			if (parent_shell != NULL)
			{
				if (parent_shell->action_group != NULL)
					unity_gtk_action_group_connect_shell(parent_shell
					                                         ->action_group,
					                                     item->child_shell);
			}
			else
				g_warn_if_reached();
		}
	}

	return item->child_shell;
}

void unity_gtk_menu_item_set_action(UnityGtkMenuItem *item, UnityGtkAction *action)
{
	UnityGtkAction *old_action;

	g_return_if_fail(UNITY_GTK_IS_MENU_ITEM(item));

	old_action = item->action;

	if (action != old_action)
	{
		if (old_action != NULL)
		{
			item->action = NULL;
			g_object_unref(old_action);
		}

		if (action != NULL)
			item->action = g_object_ref(action);
	}
}

static char *g_strdup_no_mnemonics(const char *str)
{
	if (str != NULL)
	{
		char *string;
		char *out;
		const char *in;
		gboolean underscore;

		string     = g_malloc(strlen(str) + 1);
		out        = string;
		underscore = FALSE;

		for (in = str; *in != '\0'; in++)
		{
			if (*in != '_')
			{
				underscore = FALSE;
				*out++     = *in;
			}
			else
			{
				if (!underscore)
					underscore = TRUE;
				else
				{
					/* double underscores are not accelerator markers */
					underscore = FALSE;
					*out++     = '_';
					*out++     = '_';
				}
			}
		}

		/* trailing underscores are not accelerator markers */
		if (underscore)
			*out++ = '_';

		*out++ = '\0';

		return string;
	}

	return NULL;
}

static char *g_strdup_escape_underscores(const char *str)
{
	if (str != NULL)
	{
		char *string;
		char *out;
		const char *in;
		guint underscores;

		underscores = 0;

		for (in = strchr(str, '_'); in != NULL; in = strchr(in + 1, '_'))
			underscores++;

		if (underscores == 0)
			return g_strdup(str);

		string = g_malloc(strlen(str) + underscores + 1);
		out    = string;

		for (in = str; *in != '\0'; in++)
		{
			*out++ = *in;

			if (*in == '_')
				*out++ = '_';
		}

		*out++ = '\0';

		return string;
	}

	return NULL;
}

const char *unity_gtk_menu_item_get_label(UnityGtkMenuItem *item)
{
	g_return_val_if_fail(UNITY_GTK_IS_MENU_ITEM(item), NULL);
	g_return_val_if_fail(item->menu_item != NULL, NULL);

	if (item->label_label == NULL)
	{
		const char *label_label = gtk_menu_item_get_label(item->menu_item);

		if (label_label != NULL && label_label[0] != '\0')
		{
			G_GNUC_BEGIN_IGNORE_DEPRECATIONS
			if (GTK_IS_IMAGE_MENU_ITEM(item->menu_item))
			{
				GtkImageMenuItem *image_menu_item =
				    GTK_IMAGE_MENU_ITEM(item->menu_item);

				if (gtk_image_menu_item_get_use_stock(image_menu_item))
				{
					GtkStockItem stock_item;

					if (gtk_stock_lookup(label_label, &stock_item))
						label_label = stock_item.label;
				}
			}
			G_GNUC_END_IGNORE_DEPRECATIONS
		}

		if (label_label == NULL || label_label[0] == '\0')
			label_label = gtk_menu_item_get_nth_label_label(item->menu_item, 0);

		if (label_label != NULL && label_label[0] != '\0')
		{
			GtkLabel *label = gtk_menu_item_get_nth_label(item->menu_item, 0);

			if (gtk_label_get_use_underline(label))
			{
				if (item->parent_shell == NULL || item->parent_shell->has_mnemonics)
					item->label_label = g_strdup(label_label);
				else
					item->label_label = g_strdup_no_mnemonics(label_label);
			}
			else
				item->label_label = g_strdup_escape_underscores(label_label);
		}
	}

	return item->label_label;
}

GIcon *unity_gtk_menu_item_get_icon(UnityGtkMenuItem *item)
{
	GIcon *icon = NULL;

	g_return_val_if_fail(UNITY_GTK_IS_MENU_ITEM(item), NULL);

	G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	if (item->menu_item != NULL && !GTK_IS_IMAGE_MENU_ITEM(item->menu_item))
	{
		GtkImage *image = gtk_menu_item_get_nth_image(item->menu_item, 0);

		if (image != NULL)
			icon = gtk_image_get_icon(image);
	}
	G_GNUC_END_IGNORE_DEPRECATIONS
	return icon;
}

gboolean unity_gtk_menu_item_is_visible(UnityGtkMenuItem *item)
{
	g_return_val_if_fail(UNITY_GTK_IS_MENU_ITEM(item), FALSE);
	G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	return item->menu_item != NULL && gtk_widget_get_visible(GTK_WIDGET(item->menu_item)) &&
	       !GTK_IS_TEAROFF_MENU_ITEM(item->menu_item);
	G_GNUC_END_IGNORE_DEPRECATIONS
}

gboolean unity_gtk_menu_item_is_sensitive(UnityGtkMenuItem *item)
{
	g_return_val_if_fail(UNITY_GTK_IS_MENU_ITEM(item), FALSE);

	return item->menu_item != NULL && gtk_widget_is_sensitive(GTK_WIDGET(item->menu_item));
}

gboolean unity_gtk_menu_item_is_active(UnityGtkMenuItem *item)
{
	g_return_val_if_fail(UNITY_GTK_IS_MENU_ITEM(item), FALSE);

	return GTK_IS_CHECK_MENU_ITEM(item->menu_item) &&
	       gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(item->menu_item));
}

gboolean unity_gtk_menu_item_is_separator(UnityGtkMenuItem *item)
{
	g_return_val_if_fail(UNITY_GTK_IS_MENU_ITEM(item), FALSE);

	return GTK_IS_SEPARATOR_MENU_ITEM(item->menu_item);
}

gboolean unity_gtk_menu_item_is_check(UnityGtkMenuItem *item)
{
	g_return_val_if_fail(UNITY_GTK_IS_MENU_ITEM(item), FALSE);

	return GTK_IS_CHECK_MENU_ITEM(item->menu_item);
}

gboolean unity_gtk_menu_item_is_radio(UnityGtkMenuItem *item)
{
	g_return_val_if_fail(UNITY_GTK_IS_MENU_ITEM(item), FALSE);

	return GTK_IS_RADIO_MENU_ITEM(item->menu_item);
}

gboolean unity_gtk_menu_item_get_draw_as_radio(UnityGtkMenuItem *item)
{
	g_return_val_if_fail(UNITY_GTK_IS_MENU_ITEM(item), FALSE);

	return GTK_IS_CHECK_MENU_ITEM(item->menu_item) &&
	       gtk_check_menu_item_get_draw_as_radio(GTK_CHECK_MENU_ITEM(item->menu_item));
}

void unity_gtk_menu_item_activate(UnityGtkMenuItem *item)
{
	g_return_if_fail(UNITY_GTK_IS_MENU_ITEM(item));
	g_return_if_fail(item->parent_shell != NULL);

	unity_gtk_menu_shell_activate_item(item->parent_shell, item);
}

void unity_gtk_menu_item_print(UnityGtkMenuItem *item, guint indent)
{
	char *space;

	g_return_if_fail(item == NULL || UNITY_GTK_IS_MENU_ITEM(item));

	space = g_strnfill(indent, ' ');

	if (item != NULL)
	{
		const char *label = unity_gtk_menu_item_get_label(item);

		if (label != NULL)
			g_print("%s%u (%s *) %p \"%s\"\n",
			        space,
			        item->item_index,
			        G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(item)),
			        item,
			        label);
		else
			g_print("%s%u (%s *) %p\n",
			        space,
			        item->item_index,
			        G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(item)),
			        item);

		if (item->menu_item != NULL)
			g_print("%s  (%s *) %p\n",
			        space,
			        G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(item->menu_item)),
			        item->menu_item);

		if (item->parent_shell != NULL)
			g_print("%s  (%s *) %p\n",
			        space,
			        G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(item->parent_shell)),
			        item->parent_shell);

		if (item->child_shell_valid || item->child_shell != NULL)
		{
			if (!item->child_shell_valid)
				g_print("%s  invalid\n", space);

			unity_gtk_menu_shell_print(item->child_shell, indent + 2);
		}

		if (item->action != NULL)
			unity_gtk_action_print(item->action, indent + 2);
	}
	else
		g_print("%sNULL\n", space);

	g_free(space);
}
