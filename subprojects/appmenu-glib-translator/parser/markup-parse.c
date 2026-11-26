/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* Markup parsing {{{1 */

/*
 * Inverse of g_menu_markup_print_string() (see debug.c): reconstructs a
 * GMenuModel hierarchy from menu markup. The accepted grammar is the subset the
 * printer emits plus the GtkBuilder UI-file superset:
 *
 *   model      := ( item | section | submenu )*
 *   item       := <item string-attrs* ( "/>" | ">" body "</item>" )
 *   body       := ( attribute | link )*
 *   attribute  := <attribute name='X' [type='T'] [translatable] >text</attribute>
 *   link       := <link name='submenu|section'> model </link>
 *   section    := <section [id]> model </section>
 *   submenu    := <submenu [id]> model </submenu>
 *
 * String attributes are emitted inline on the <item> tag and come back as
 * strings; non-string attributes become <attribute> children whose text is a
 * GVariant (when type= is given) or a plain string (UI-file form, no type=). A
 * <link name='submenu|section'> recurses into a fresh model.
 *
 * UI-file <section>/<submenu> mirror the link form: each becomes its own nested
 * GMenu wrapped in an item linked as "section"/"submenu", so empty groups are
 * preserved rather than collapsed. translatable="yes" attribute text is run
 * through dgettext() with the parser's gettext package (kept verbatim when no
 * package is set).
 *
 * An element's id= becomes a menu item attribute (see
 * DBUS_MENU_ATTRIBUTE_TRANSLATOR_ID), which is how a parsed tree can be searched
 * for a handle later without GTK. It applies to every element that produces an
 * item: plain <item> inline attrs are free-form, so id= is renamed there, while
 * the section/submenu wrapper items get it from the id= captured at their start
 * tag. A <link> keeps its own name= and never gets the attribute.
 *
 * Parsing uses GLib's GMarkupParser event API: XML well-formedness, entity
 * unescaping and attribute tokenising come for free. A frame stack (as in GTK's
 * gtkbuilder-menus.c) tracks the model/item being filled; a <item> tag's
 * free-form string attributes are read straight from the names/values arrays
 * g_markup_collect_attributes() cannot enumerate unknown names, so we iterate
 * them directly, exactly as GtkBuilder does.
 */

#include "parser.h"
#include "definitions.h"
#include <libintl.h>
#include <stdarg.h>
#include <string.h>

#define DBUS_MENU_ATTRIBUTE_TRANSLATOR_ID "x-translator-id"

/* Parser state ------------------------------------------------------------ */

/* A frame records the model and item we are currently filling. Opening a scope
 * (item body, link, section or submenu) pushes the previous frame so that, on
 * close, an item created in this scope is appended to its parent model. This
 * mirrors the frame stack in GTK's gtkbuilder-menus.c. */
struct frame
{
	GMenu *menu;     /* model receiving items at this level, or NULL */
	GMenuItem *item; /* item being filled at this level, or NULL */
	struct frame *prev;
};

typedef struct
{
	GMarkupParseContext *ctx;
	const char *gettext_package;
	struct frame frame;
	/* in-progress <attribute> state (NULL/zero when not inside one) */
	char *attr_name;
	GVariantType *attr_type; /* NULL => plain-string UI-file form */
	char *attr_type_str;     /* owned copy of type=, for diagnostics */
	gboolean attr_translatable;
	GString *attr_text;
} MarkupParser;

static void push_frame(MarkupParser *ps, GMenu *menu, GMenuItem *item)
{
	struct frame *prev = g_new(struct frame, 1);
	*prev              = ps->frame;
	ps->frame.menu     = menu;
	ps->frame.item     = item;
	ps->frame.prev     = prev;
}

/* Append the frame's item to the parent model and pop. */
static void pop_frame(MarkupParser *ps)
{
	struct frame *prev = ps->frame.prev;

	if (ps->frame.item != NULL && prev->menu != NULL)
	{
		/* append takes its own ref but does not sink ours; drop it explicitly. */
		g_menu_append_item(prev->menu, ps->frame.item);
		g_object_unref(ps->frame.item);
	}

	ps->frame = *prev;
	g_free(prev);
}

/* A model scope always has a menu; an item body (menu==NULL) does not. A
 * section/submenu frame has BOTH a menu and its wrapper item, so the menu is
 * what distinguishes the two scopes - not whether an item is present. */
static gboolean in_item(MarkupParser *ps)
{
	return ps->frame.menu == NULL;
}

/* Error reporting. Reports carry a line/column from the parse context. We never
 * overwrite an already-set error: once one callback fails, GMarkup keeps firing
 * remaining callbacks for the same element before it unwinds, so guarding here
 * both avoids "GError set over previous" warnings and keeps the first (root)
 * cause as the reported error. */
static void fail(MarkupParser *ps, GError **error, const char *fmt, ...) G_GNUC_PRINTF(3, 4);
static void fail(MarkupParser *ps, GError **error, const char *fmt, ...)
{
	if (error == NULL || *error != NULL)
		return;

	va_list ap;
	va_start(ap, fmt);
	char *detail = g_strdup_vprintf(fmt, ap);
	va_end(ap);

	int line = 0, col = 0;
	g_markup_parse_context_get_position(ps->ctx, &line, &col);

	g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
	            "Malformed menu markup at line %d col %d: %s", line, col, detail);
	g_free(detail);
}

/* Element handlers -------------------------------------------------------- */

static void start_item(MarkupParser *ps, const char **names, const char **values, GError **error)
{
	if (in_item(ps)) /* <item> appearing where an item body is expected */
	{
		fail(ps, error, "<item> not allowed inside an item");
		return;
	}

	GMenuItem *item = g_menu_item_new(NULL, NULL);

	/* Inline attrs are free-form string attributes. g_markup_collect_attributes
	 * cannot enumerate unknown names, so iterate the pairs directly (as
	 * GtkBuilder does). "translatable" is a meta-attribute, not a menu one; "id"
	 * is GtkBuilder's handle and is stored under its own attribute name so it can
	 * be found with dbus_menu_parsed_find_item_by_id(). */
	for (const char **p = names; *p != NULL; p++)
	{
		guint i = (guint)(p - names);
		const char *nm = *p;

		if (strcmp(nm, "translatable") == 0)
			continue;

		const char *attr = strcmp(nm, "id") == 0 ? DBUS_MENU_ATTRIBUTE_TRANSLATOR_ID : nm;

		GVariant *v = g_variant_ref_sink(g_variant_new_string(values[i]));
		g_menu_item_set_attribute_value(item, attr, v);
		g_variant_unref(v); /* set_attribute_value does not sink our ref */
	}

	push_frame(ps, NULL, item);
}

static void start_group(MarkupParser *ps, const char **names, const char **values,
                        gboolean is_submenu)
{
	GMenu *child = g_menu_new();
	GMenuItem *gitem = is_submenu ? g_menu_item_new_submenu(NULL, G_MENU_MODEL(child))
	                              : g_menu_item_new_section(NULL, G_MENU_MODEL(child));

	for (const char **p = names; *p != NULL; p++)
	{
		guint i = (guint)(p - names);

		if (strcmp(*p, "id") == 0)
		{
			GVariant *v = g_variant_ref_sink(g_variant_new_string(values[i]));
			g_menu_item_set_attribute_value(gitem, DBUS_MENU_ATTRIBUTE_TRANSLATOR_ID, v);
			g_variant_unref(v);
		}
		/* GtkBuilder groups carry no other attributes; ignore the rest. */
	}

	push_frame(ps, child, gitem);
	g_object_unref(child); /* the wrapper item holds the model ref; appended on pop */
}

static void start_attribute(MarkupParser *ps, const char **names, const char **values, GError **error)
{
	if (!in_item(ps))
	{
		fail(ps, error, "<attribute> must be inside an <item>");
		return;
	}

	const char *name = NULL, *type = NULL, *translatable = NULL;
	gboolean seen_name = FALSE, seen_type = FALSE, seen_trans = FALSE;

	for (const char **p = names; *p != NULL; p++)
	{
		guint i       = (guint)(p - names);
		const char *nm = *p;

		if (strcmp(nm, "name") == 0)
		{
			if (seen_name) { fail(ps, error, "duplicate 'name' attribute on <attribute>"); return; }
			seen_name = TRUE; name = values[i];
		}
		else if (strcmp(nm, "type") == 0)
		{
			if (seen_type) { fail(ps, error, "duplicate 'type' attribute on <attribute>"); return; }
			seen_type = TRUE; type = values[i];
		}
		else if (strcmp(nm, "translatable") == 0)
		{
			if (seen_trans) { fail(ps, error, "duplicate 'translatable' attribute on <attribute>"); return; }
			seen_trans = TRUE; translatable = values[i];
		}
		else if (strcmp(nm, "context") == 0 || strcmp(nm, "comments") == 0)
			; /* known-but-unused */
		else
		{
			fail(ps, error, "unknown attribute '%s' on <attribute>", nm);
			return;
		}
	}

	if (name == NULL) { fail(ps, error, "<attribute> requires a 'name' attribute"); return; }

	if (type != NULL && !g_variant_type_string_is_valid(type))
	{
		fail(ps, error, "attribute '%s' has invalid GVariant type string '%s'", name, type);
		return;
	}

	ps->attr_name         = g_strdup(name);
	ps->attr_translatable = translatable && strcmp(translatable, "yes") == 0;
	ps->attr_type         = type ? g_variant_type_new(type) : NULL;
	ps->attr_type_str     = type ? g_strdup(type) : NULL;
	ps->attr_text         = g_string_new(NULL);

	push_frame(ps, NULL, NULL); /* sentinel frame so text() routes into attr_text */
}

static void start_link(MarkupParser *ps, const char **names, const char **values, GError **error)
{
	if (!in_item(ps)) { fail(ps, error, "<link> must be inside an <item>"); return; }

	const char *name = NULL;
	gboolean seen = FALSE;

	for (const char **p = names; *p != NULL; p++)
	{
		guint i = (guint)(p - names);

		if (strcmp(*p, "name") == 0)
		{
			if (seen) { fail(ps, error, "duplicate 'name' attribute on <link>"); return; }
			seen = TRUE; name = values[i];
		}
		else if (strcmp(*p, "id") == 0)
			; /* grouping handle */
		else { fail(ps, error, "unknown attribute '%s' on <link>", *p); return; }
	}

	if (name == NULL) { fail(ps, error, "<link> requires a 'name' attribute"); return; }

	GMenu *child = g_menu_new();
	g_menu_item_set_link(ps->frame.item, name, G_MENU_MODEL(child));
	push_frame(ps, child, NULL); /* item==NULL: pop appends nothing */
	g_object_unref(child);       /* the link holds the model ref */
}

static void start_element(GMarkupParseContext *c, const char *element,
                          const char **names, const char **values,
                          gpointer user, GError **error)
{
	MarkupParser *ps = user;
	(void)c;

	if (strcmp(element, "item") == 0)      { start_item(ps, names, values, error); return; }
	if (strcmp(element, "attribute") == 0) { start_attribute(ps, names, values, error); return; }
	if (strcmp(element, "link") == 0)      { start_link(ps, names, values, error); return; }

	if (in_item(ps))
	{
		fail(ps, error, "<%s> not allowed inside an item", element);
		return;
	}

	if (strcmp(element, "section") == 0) { start_group(ps, names, values, FALSE); return; }
	if (strcmp(element, "submenu") == 0) { start_group(ps, names, values, TRUE); return; }

	fail(ps, error, "element <%s> not allowed here", element);
}

/* Finish an in-progress <attribute>: build the value from its collected text. */
static void end_attribute(MarkupParser *ps, GError **error)
{
	char *text = g_string_free(ps->attr_text, FALSE);
	ps->attr_text = NULL;

	/* translatable text is a msgid; run it through the parser's gettext package
	 * (kept verbatim when no package is bound). */
	if (ps->attr_translatable && ps->gettext_package != NULL)
	{
		char *tr = dgettext(ps->gettext_package, text);

		if (tr != text)
		{
			g_free(text);
			text = g_strdup(tr);
		}
	}

	GMenuItem *item  = ps->frame.prev->item; /* the item being annotated */
	GVariant *value  = NULL;

	if (ps->attr_type == NULL) /* UI-file plain-string form (no type=) */
		value = g_variant_ref_sink(g_variant_new_string(text));

	else if (!g_variant_type_is_definite(ps->attr_type))
		fail(ps, error, "attribute '%s' has non-definite GVariant type", ps->attr_name);

	else
	{
		GError *local = NULL;
		value = g_variant_parse(ps->attr_type, text, NULL, NULL, &local);

		if (value == NULL)
		{
			fail(ps, error, "attribute '%s': cannot parse GVariant of type '%s': %s",
			     ps->attr_name, ps->attr_type_str, local ? local->message : "unknown error");
			g_clear_error(&local);
		}
	}

	if (value != NULL)
	{
		g_menu_item_set_attribute_value(item, ps->attr_name, value);
		g_variant_unref(value); /* set_attribute_value does not sink our ref */
	}

	g_free(text);
	g_clear_pointer(&ps->attr_type, g_variant_type_free);
	g_clear_pointer(&ps->attr_type_str, g_free);
	g_clear_pointer(&ps->attr_name, g_free);
	pop_frame(ps);
}

static void end_element(GMarkupParseContext *c, const char *element, gpointer user, GError **error)
{
	MarkupParser *ps = user;
	(void)c;

	if (ps->attr_text != NULL && strcmp(element, "attribute") == 0)
	{
		end_attribute(ps, error);
		return;
	}

	pop_frame(ps); /* item / section / submenu / link */
}

static void on_text(GMarkupParseContext *c, const char *t, gsize n, gpointer user, GError **error)
{
	MarkupParser *ps = user;
	(void)c;

	if (ps->attr_text != NULL) /* inside <attribute>: it is the value text */
	{
		g_string_append_len(ps->attr_text, t, n);
		return;
	}

	/* outside any attribute only inter-element whitespace is allowed */
	for (gsize i = 0; i < n; i++)
	{
		if (!g_ascii_isspace(t[i]))
		{
			fail(ps, error, "text not allowed here");
			return;
		}
	}
}

/* Free frames / attribute state orphaned when GMarkup aborts mid-element (it
 * does not call end_element for the element being processed once an error is
 * set). Must run before the context's last reference is dropped. */
static void cleanup(MarkupParser *ps)
{
	if (ps->attr_text) { g_string_free(ps->attr_text, TRUE); ps->attr_text = NULL; }
	g_clear_pointer(&ps->attr_type, g_variant_type_free);
	g_clear_pointer(&ps->attr_type_str, g_free);
	g_clear_pointer(&ps->attr_name, g_free);

	while (ps->frame.prev)
	{
		struct frame *prev = ps->frame.prev;
		if (ps->frame.item) g_object_unref(ps->frame.item); /* drop un-appended item */
		ps->frame = *prev; /* copy BEFORE freeing prev */
		g_free(prev);
	}
}

GMenuModel *dbus_menu_parse_one(const char *markup, const char *gettext_package, GError **error)
{
	g_return_val_if_fail(markup != NULL, NULL);

	static const GMarkupParser handlers = {
		start_element, end_element, on_text, NULL, NULL
	};

	MarkupParser ps = {0};
	ps.gettext_package = gettext_package;

	GMenu *root = g_menu_new();
	ps.frame.menu     = root;
	ps.frame.item     = NULL;
	ps.frame.prev     = NULL;

	ps.ctx = g_markup_parse_context_new(&handlers, 0, &ps, NULL);

	gboolean ok = g_markup_parse_context_parse(ps.ctx, markup, -1, error) &&
	              g_markup_parse_context_end_parse(ps.ctx, error);

	if (!ok)
	{
		cleanup(&ps);
		g_markup_parse_context_free(ps.ctx);
		g_object_unref(root);
		return NULL;
	}

	g_markup_parse_context_free(ps.ctx);
	return G_MENU_MODEL(root);
}

/* Builds a standalone GMenuItem that mirrors item @i of @model: its scalar
 * attributes are copied, but its submenu/section links point at the very same
 * sub-models the model already holds (borrowed refs - no copies). That is what
 * lets an id name either an ordinary item or a group and still let the caller
 * walk/edit the real tree behind it. g_menu_item_new_from_model() cannot be used
 * here because its link table owns its sub-models, whereas we want to keep only
 * borrowed pointers to the live ones (the container stays with the model). */
static GMenuItem *item_wrapper_borrowing_links(GMenuModel *model, int i)
{
	GMenuItem *item = g_menu_item_new(NULL, NULL);

	GMenuAttributeIter *attrs = g_menu_model_iterate_item_attributes(model, i);
	const char *name;
	GVariant *value;
	while (g_menu_attribute_iter_get_next(attrs, &name, &value))
	{
		/* set_attribute_value takes its own ref; drop the one get_next handed us. */
		g_menu_item_set_attribute_value(item, name, value);
		g_variant_unref(value);
	}
	g_object_unref(attrs);

	GMenuLinkIter *links = g_menu_model_iterate_item_links(model, i);
	const char *link_name;
	GMenuModel *sub;
	while (g_menu_link_iter_get_next(links, &link_name, &sub))
	{
		/* set_link takes its own ref; the returned sub-model ref is ours to drop,
		 * so the wrapper keeps only borrowed pointers to the live sub-models. */
		g_menu_item_set_link(item, link_name, sub);
		g_object_unref(sub);
	}
	g_object_unref(links);

	return item;
}

/* Depth-first search for the item whose id attribute (set from a markup element's
 * id=, see DBUS_MENU_ATTRIBUTE_TRANSLATOR_ID) equals @id. A section/submenu is
 * matched through the item that links it, never by naming the model behind the
 * link - that keeps one lookup rule for every kind of element.
 *
 * The returned GMenuItem is new (transfer container): its own attributes are a
 * private copy, but its submenu/section links are the same live sub-models the
 * tree holds, so the caller can reach and modify the real groups behind them. It
 * borrows those sub-models, so it must not outlive the model it was found in. */
GMenuItem *dbus_menu_parsed_find_item_by_id(GMenuModel *model, const char *id)
{
	if (!model || !id)
		return NULL;

	int n = g_menu_model_get_n_items(model);

	for (int i = 0; i < n; i++)
	{
		g_autofree char *item_id = NULL;

		if (g_menu_model_get_item_attribute(model, i, DBUS_MENU_ATTRIBUTE_TRANSLATOR_ID, "s", &item_id) &&
		    g_strcmp0(item_id, id) == 0)
			return item_wrapper_borrowing_links(model, i);

		/* Recurse into submenu and section links (the only two GMenuLinkIter can
		 * yield; an item may carry both). Also handle our DBUS_MENU_DISABLED_SUBMENU links */
		for (int l = 0; l < 3; l++)
		{
			const char	*link;
			switch (l) {
				case 0:
					link = G_MENU_LINK_SUBMENU;
					break;
				case 1:
					link = G_MENU_LINK_SECTION;
					break;
				default:
					link = DBUS_MENU_DISABLED_SUBMENU;
			}
			GMenuModel *sub  = g_menu_model_get_item_link(model, i, link);

			if (sub == NULL)
				continue;

			GMenuItem *found = dbus_menu_parsed_find_item_by_id(sub, id);
			g_object_unref(sub);

			if (found)
				return found;
		}
	}

	return NULL;
}
