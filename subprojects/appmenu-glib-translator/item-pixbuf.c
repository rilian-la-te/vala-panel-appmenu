#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gio/gio.h>

G_GNUC_INTERNAL GIcon *g_icon_new_pixbuf_from_variant(GVariant *variant)
{
	gsize length;
	const unsigned char *data =
	    (const unsigned char *)g_variant_get_fixed_array(variant, &length, sizeof(guchar));
	if (length == 0)
		return NULL;

	g_autoptr(GInputStream) stream = g_memory_input_stream_new_from_data(data, length, NULL);
	if (stream == NULL)
		return NULL;

	g_autoptr(GError) error = NULL;
	GdkPixbuf *pixbuf       = gdk_pixbuf_new_from_stream(stream, NULL, &error);
	if (error != NULL)
		g_warning("Unable to build GdkPixbuf from icon data: %s", error->message);

	return G_ICON(pixbuf);
}
