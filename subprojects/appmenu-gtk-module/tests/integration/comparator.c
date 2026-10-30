/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include "comparator.h"

static gboolean item_has_attributes(GMenuModel *model, int item_index)
{
	GMenuAttributeIter *iter = g_menu_model_iterate_item_attributes(model, item_index);
	gboolean has             = FALSE;
	const char *name;
	GVariant *value;

	while (g_menu_attribute_iter_get_next(iter, &name, &value))
	{
		has = TRUE;
		g_variant_unref(value);
		break;
	}

	g_object_unref(iter);
	return has;
}

/*
 * If @model consists of exactly one item that is a pure section wrapper
 * (only a section link, no attributes, no submenu), return the wrapped
 * section with a new reference.  Otherwise return @model with a new
 * reference.
 *
 * The parser always wraps items in sections, so this lets a flat GMenu
 * (like the submenus produced by g_menu_append_submenu) compare equal to
 * the parsed, section-wrapped equivalent.
 */
static GMenuModel *unwrap_single_section(GMenuModel *model)
{
	if (g_menu_model_get_n_items(model) != 1)
		return g_object_ref(model);

	GMenuModel *sec = g_menu_model_get_item_link(model, 0, G_MENU_LINK_SECTION);
	if (sec == NULL)
		return g_object_ref(model);

	/* Not a pure wrapper if there is also a submenu link. */
	GMenuModel *sub = g_menu_model_get_item_link(model, 0, G_MENU_LINK_SUBMENU);
	if (sub != NULL)
	{
		g_object_unref(sub);
		g_object_unref(sec);
		return g_object_ref(model);
	}

	/* Not a pure wrapper if it has any attributes. */
	if (item_has_attributes(model, 0))
	{
		g_object_unref(sec);
		return g_object_ref(model);
	}

	return sec;
}

static gboolean item_is_container(GMenuModel *m, int i)
{
	GMenuModel *sub    = g_menu_model_get_item_link(m, i, G_MENU_LINK_SUBMENU);
	GMenuModel *sec    = g_menu_model_get_item_link(m, i, G_MENU_LINK_SECTION);
	gboolean container = (sub != NULL || sec != NULL);
	g_clear_object(&sub);
	g_clear_object(&sec);
	return container;
}

static gboolean compare_models_raw(GMenuModel *a, GMenuModel *b, GString *errors, int depth)
{
	int na      = g_menu_model_get_n_items(a);
	int nb      = g_menu_model_get_n_items(b);
	gboolean ok = TRUE;

	if (na != nb)
	{
		g_string_append_printf(errors,
		                       "%*sn_items mismatch: %d vs %d\n",
		                       depth * 2,
		                       "",
		                       na,
		                       nb);
		return FALSE;
	}

	for (int i = 0; i < na; i++)
	{
		/* label */
		const char *la = NULL;
		const char *lb = NULL;
		g_menu_model_get_item_attribute(a, i, G_MENU_ATTRIBUTE_LABEL, "s", &la);
		g_menu_model_get_item_attribute(b, i, G_MENU_ATTRIBUTE_LABEL, "s", &lb);

		if (g_strcmp0(la, lb) != 0)
		{
			g_string_append_printf(errors,
			                       "%*sitem %d label mismatch: '%s' vs '%s'\n",
			                       depth * 2,
			                       "",
			                       i,
			                       la ? la : "(null)",
			                       lb ? lb : "(null)");
			ok = FALSE;
		}

		gboolean cont_a = item_is_container(a, i);
		gboolean cont_b = item_is_container(b, i);

		if (cont_a != cont_b)
		{
			g_string_append_printf(errors,
			                       "%*sitem %d container mismatch (section/submenu)\n",
			                       depth * 2,
			                       "",
			                       i);
			ok = FALSE;
		}

		if (!cont_a && !cont_b)
		{
			GVariant *act_a =
			    g_menu_model_get_item_attribute_value(a,
			                                          i,
			                                          G_MENU_ATTRIBUTE_ACTION,
			                                          NULL);
			GVariant *act_b =
			    g_menu_model_get_item_attribute_value(b,
			                                          i,
			                                          G_MENU_ATTRIBUTE_ACTION,
			                                          NULL);
			if ((act_a == NULL) != (act_b == NULL))
			{
				g_string_append_printf(errors,
				                       "%*sitem %d action presence mismatch\n",
				                       depth * 2,
				                       "",
				                       i);
				ok = FALSE;
			}
			g_clear_pointer(&act_a, g_variant_unref);
			g_clear_pointer(&act_b, g_variant_unref);

			GVariant *tgt_a =
			    g_menu_model_get_item_attribute_value(a,
			                                          i,
			                                          G_MENU_ATTRIBUTE_TARGET,
			                                          NULL);
			GVariant *tgt_b =
			    g_menu_model_get_item_attribute_value(b,
			                                          i,
			                                          G_MENU_ATTRIBUTE_TARGET,
			                                          NULL);
			if ((tgt_a == NULL) != (tgt_b == NULL))
			{
				g_string_append_printf(errors,
				                       "%*sitem %d target presence mismatch\n",
				                       depth * 2,
				                       "",
				                       i);
				ok = FALSE;
			}
			g_clear_pointer(&tgt_a, g_variant_unref);
			g_clear_pointer(&tgt_b, g_variant_unref);
		}

		/* submenu */
		GMenuModel *sub_a = g_menu_model_get_item_link(a, i, G_MENU_LINK_SUBMENU);
		GMenuModel *sub_b = g_menu_model_get_item_link(b, i, G_MENU_LINK_SUBMENU);

		if ((sub_a == NULL) != (sub_b == NULL))
		{
			g_string_append_printf(errors,
			                       "%*sitem %d submenu presence mismatch\n",
			                       depth * 2,
			                       "",
			                       i);
			ok = FALSE;
		}
		else if (sub_a != NULL && !compare_models(sub_a, sub_b, errors, depth + 1))
			ok = FALSE;

		g_clear_object(&sub_a);
		g_clear_object(&sub_b);

		/* section */
		GMenuModel *sec_a = g_menu_model_get_item_link(a, i, G_MENU_LINK_SECTION);
		GMenuModel *sec_b = g_menu_model_get_item_link(b, i, G_MENU_LINK_SECTION);

		if ((sec_a == NULL) != (sec_b == NULL))
		{
			g_string_append_printf(errors,
			                       "%*sitem %d section presence mismatch\n",
			                       depth * 2,
			                       "",
			                       i);
			ok = FALSE;
		}
		else if (sec_a != NULL && !compare_models(sec_a, sec_b, errors, depth + 1))
			ok = FALSE;

		g_clear_object(&sec_a);
		g_clear_object(&sec_b);
	}

	return ok;
}

gboolean compare_models(GMenuModel *a, GMenuModel *b, GString *errors, int depth)
{
	GMenuModel *ua = unwrap_single_section(a);
	GMenuModel *ub = unwrap_single_section(b);
	gboolean ok    = compare_models_raw(ua, ub, errors, depth);
	g_object_unref(ua);
	g_object_unref(ub);
	return ok;
}

void check_any_actions(GMenuModel *model, gboolean *found)
{
	int n = g_menu_model_get_n_items(model);

	for (int i = 0; i < n && !*found; i++)
	{
		char *action = NULL;
		if (g_menu_model_get_item_attribute(model,
		                                    i,
		                                    G_MENU_ATTRIBUTE_ACTION,
		                                    "s",
		                                    &action) &&
		    action != NULL)
		{
			if (g_str_has_prefix(action, "unity."))
				*found = TRUE;
		}
		g_free(action);

		GMenuModel *sub = g_menu_model_get_item_link(model, i, G_MENU_LINK_SUBMENU);
		if (sub != NULL)
		{
			check_any_actions(sub, found);
			g_object_unref(sub);
		}

		GMenuModel *sec = g_menu_model_get_item_link(model, i, G_MENU_LINK_SECTION);
		if (sec != NULL)
		{
			check_any_actions(sec, found);
			g_object_unref(sec);
		}
	}
}
