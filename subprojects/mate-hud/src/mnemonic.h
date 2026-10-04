#pragma once
#include <glib.h>

/*
 * Parse a GTK mnemonic label (underscores as mnemonic prefix) and produce:
 *   markup  — Pango markup string with the mnemonic char wrapped in <u>…</u>
 *             All special Pango chars (& < >) are escaped first.
 *   plain   — plain text with mnemonics and underscores stripped, for search.
 *
 * The caller owns both returned strings; free with g_free().
 * Either out-pointer may be NULL if that output is not needed.
 */
void mnemonic_parse(const char *label, char **markup, char **plain);

/*
 * Build the full path markup for a menu entry from a NULL-terminated array of
 * segment labels (e.g. {"_File", "_Open", NULL}).
 * Segments are separated with " ▸ ".
 * Returns a Pango markup string; caller owns it.
 */
char *mnemonic_build_path_markup(const char * const *segments, int n_segments);

/*
 * Same but returns plain text (no markup, no underscores) for search matching.
 */
char *mnemonic_build_path_plain(const char * const *segments, int n_segments);
