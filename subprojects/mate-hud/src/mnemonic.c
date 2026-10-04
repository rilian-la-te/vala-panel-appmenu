#include "mnemonic.h"
#include <glib.h>
#include <string.h>

/* Path separator shown between menu levels */
#define SEP " \xe2\x96\xb8 "  /* U+25B8 BLACK RIGHT-POINTING SMALL TRIANGLE */

/*
 * Scan one mnemonic label.
 * Rules:
 *   "__"  → literal "_" in output, no mnemonic
 *   "_X"  → X is the mnemonic character; omit the underscore from output
 *   any other char → emitted as-is
 *
 * We produce two parallel strings:
 *   plain_buf   — raw text, no markup, mnemonics stripped
 *   markup_buf  — Pango-escaped text with mnemonic wrapped in <u>…</u>
 *
 * Pango escaping must happen BEFORE inserting markup tags; otherwise a label
 * like "Save & Quit" would produce broken markup "Save &amp; Quit" if we
 * escape after inserting tags. We escape per-character as we go.
 */
void mnemonic_parse(const char *label, char **out_markup, char **out_plain)
{
    if (!label) {
        if (out_markup) *out_markup = g_strdup("");
        if (out_plain)  *out_plain  = g_strdup("");
        return;
    }

    GString *markup = out_markup ? g_string_new(NULL) : NULL;
    GString *plain  = out_plain  ? g_string_new(NULL) : NULL;
    gboolean mnemonic_pending = FALSE;

    for (const char *p = label; *p; ) {
        gunichar ch = g_utf8_get_char(p);
        int char_len = g_utf8_next_char(p) - p;

        if (ch == '_') {
            const char *next = g_utf8_next_char(p);
            gunichar next_ch = *next ? g_utf8_get_char(next) : 0;

            if (next_ch == '_') {
                /* "__" → literal underscore, no mnemonic */
                if (plain)  g_string_append_c(plain, '_');
                if (markup) g_string_append(markup, "_");
                p = g_utf8_next_char(next);
                continue;
            } else if (next_ch) {
                /* "_X" → X is mnemonic */
                mnemonic_pending = TRUE;
                p = next;
                continue;
            }
            /* trailing lone underscore → emit as literal */
        }

        /* Emit this character */
        char utf8[7];
        int len = g_unichar_to_utf8(ch, utf8);
        utf8[len] = '\0';

        if (plain) g_string_append(plain, utf8);

        if (markup) {
            /* Escape for Pango */
            char *escaped = g_markup_escape_text(utf8, len);
            if (mnemonic_pending) {
                g_string_append(markup, "<u>");
                g_string_append(markup, escaped);
                g_string_append(markup, "</u>");
                mnemonic_pending = FALSE;
            } else {
                g_string_append(markup, escaped);
            }
            g_free(escaped);
        } else {
            mnemonic_pending = FALSE;
        }

        p += char_len;
    }

    if (out_markup) *out_markup = g_string_free(markup, FALSE);
    if (out_plain)  *out_plain  = g_string_free(plain,  FALSE);
}

char *mnemonic_build_path_markup(const char * const *segments, int n_segments)
{
    GString *result = g_string_new(NULL);
    for (int i = 0; i < n_segments; i++) {
        if (i > 0) g_string_append(result, SEP);
        char *seg_markup = NULL;
        mnemonic_parse(segments[i], &seg_markup, NULL);
        g_string_append(result, seg_markup);
        g_free(seg_markup);
    }
    return g_string_free(result, FALSE);
}

char *mnemonic_build_path_plain(const char * const *segments, int n_segments)
{
    GString *result = g_string_new(NULL);
    for (int i = 0; i < n_segments; i++) {
        if (i > 0) g_string_append_c(result, ' ');
        char *seg_plain = NULL;
        mnemonic_parse(segments[i], NULL, &seg_plain);
        g_string_append(result, seg_plain);
        g_free(seg_plain);
    }
    return g_string_free(result, FALSE);
}
