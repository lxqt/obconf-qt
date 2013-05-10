/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   appearance.c for ObConf, the configuration tool for Openbox
   Copyright (c) 2003-2007   Dana Jansens
   Copyright (c) 2003        Tim Riley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   See the COPYING file for a copy of the GNU General Public License.
*/

#include "main.h"
#include "tree.h"
#include "preview_update.h"

static gboolean mapping = FALSE;

static RrFont *read_font(GtkFontButton *w, const gchar *place, gboolean def);
static RrFont *write_font(GtkFontButton *w, const gchar *place);

void appearance_setup_tab()
{
    GtkWidget *w;
    gchar *layout;
    RrFont *f;

    mapping = TRUE;

    w = get_widget("window_border");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w),
                                 tree_get_bool("theme/keepBorder", TRUE));

    w = get_widget("animate_iconify");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w),
                                 tree_get_bool("theme/animateIconify", TRUE));

    w = get_widget("title_layout");
    layout = tree_get_string("theme/titleLayout", "NLIMC");
    gtk_entry_set_text(GTK_ENTRY(w), layout);
    preview_update_set_title_layout(layout);
    g_free(layout);

    w = get_widget("font_active");
    f = read_font(GTK_FONT_BUTTON(w), "ActiveWindow", TRUE);
    preview_update_set_active_font(f);

    w = get_widget("font_inactive");
    f = read_font(GTK_FONT_BUTTON(w), "InactiveWindow", TRUE);
    preview_update_set_inactive_font(f);

    w = get_widget("font_menu_header");
    f = read_font(GTK_FONT_BUTTON(w), "MenuHeader", TRUE);
    preview_update_set_menu_header_font(f);

    w = get_widget("font_menu_item");
    f = read_font(GTK_FONT_BUTTON(w), "MenuItem", TRUE);
    preview_update_set_menu_item_font(f);

    w = get_widget("font_active_display");
    if (!(f = read_font(GTK_FONT_BUTTON(w), "ActiveOnScreenDisplay", FALSE))) {
        f = read_font(GTK_FONT_BUTTON(w), "OnScreenDisplay", TRUE);
        tree_delete_node("theme/font:place=OnScreenDisplay");
    }
    preview_update_set_osd_active_font(f);

    w = get_widget("font_inactive_display");
    f = read_font(GTK_FONT_BUTTON(w), "InactiveOnScreenDisplay", TRUE);
    preview_update_set_osd_inactive_font(f);

    mapping = FALSE;
}

void on_window_border_toggled(GtkToggleButton *w, gpointer data)
{
    gboolean b;

    if (mapping) return;

    b = gtk_toggle_button_get_active(w);
    tree_set_bool("theme/keepBorder", b);
}

void on_animate_iconify_toggled(GtkToggleButton *w, gpointer data)
{
    gboolean b;

    if (mapping) return;

    b = gtk_toggle_button_get_active(w);
    tree_set_bool("theme/animateIconify", b);
}

void on_title_layout_changed(GtkEntry *w, gpointer data)
{
    gchar *layout;
    gchar *it, *it2;
    gboolean n, d, s, l, i, m, c;

    if (mapping) return;

    layout = g_strdup(gtk_entry_get_text(w));

    n = d = s = l = i = m = c = FALSE;

    for (it = layout; *it; ++it) {
        gboolean *b;

        switch (*it) {
        case 'N':
        case 'n':
            b = &n;
            break;
        case 'd':
        case 'D':
            b = &d;
            break;
        case 's':
        case 'S':
            b = &s;
            break;
        case 'l':
        case 'L':
            b = &l;
            break;
        case 'i':
        case 'I':
            b = &i;
            break;
        case 'm':
        case 'M':
            b = &m;
            break;
        case 'c':
        case 'C':
            b = &c;
            break;
        default:
            b = NULL;
            break;
        }

        if (!b || *b) {
            /* drop the letter */
            for (it2 = it; *it2; ++it2)
                *it2 = *(it2+1);
        } else {
            *it = toupper(*it);
            *b = TRUE;
        }
    }

    gtk_entry_set_text(w, layout);
    tree_set_string("theme/titleLayout", layout);
    preview_update_set_title_layout(layout);

    g_free(layout);
}

void on_font_active_font_set(GtkFontButton *w, gpointer data)
{
    if (mapping) return;

    preview_update_set_active_font(write_font(w, "ActiveWindow"));
}

void on_font_inactive_font_set(GtkFontButton *w, gpointer data)
{
    if (mapping) return;

    preview_update_set_inactive_font(write_font(w, "InactiveWindow"));
}

void on_font_menu_header_font_set(GtkFontButton *w, gpointer data)
{
    if (mapping) return;

    preview_update_set_menu_header_font(write_font(w, "MenuHeader"));
}

void on_font_menu_item_font_set(GtkFontButton *w, gpointer data)
{
    if (mapping) return;

    preview_update_set_menu_item_font(write_font(w, "MenuItem"));
}

void on_font_active_display_font_set(GtkFontButton *w, gpointer data)
{
    if (mapping) return;

    preview_update_set_osd_active_font(write_font(w, "ActiveOnScreenDisplay"));
}

void on_font_inactive_display_font_set(GtkFontButton *w, gpointer data)
{
    if (mapping) return;

    preview_update_set_osd_inactive_font
        (write_font(w, "InactiveOnScreenDisplay"));
}

static RrFont *read_font(GtkFontButton *w, const gchar *place,
                         gboolean use_default)
{
    RrFont *font;
    gchar *fontstring, *node;
    gchar *name, **names;
    gchar *size;
    gchar *weight;
    gchar *slant;

    RrFontWeight rr_weight = RR_FONTWEIGHT_NORMAL;
    RrFontSlant rr_slant = RR_FONTSLANT_NORMAL;

    mapping = TRUE;

    node = g_strdup_printf("theme/font:place=%s/name", place);
    name = tree_get_string(node, use_default ? "Sans" : NULL);
    g_free(node);

    if (name[0] == '\0') {
        g_free(name);
        return NULL;
    }

    node = g_strdup_printf("theme/font:place=%s/size", place);
    size = tree_get_string(node, "8");
    g_free(node);

    node = g_strdup_printf("theme/font:place=%s/weight", place);
    weight = tree_get_string(node, "");
    g_free(node);

    node = g_strdup_printf("theme/font:place=%s/slant", place);
    slant = tree_get_string(node, "");
    g_free(node);

    /* get only the first font in the string */
    names = g_strsplit(name, ",", 0);
    g_free(name);
    name = g_strdup(names[0]);
    g_strfreev(names);

    /* don't use "normal" in the gtk string */
    if (!g_ascii_strcasecmp(weight, "normal")) {
        g_free(weight); weight = g_strdup("");
    }
    if (!g_ascii_strcasecmp(slant, "normal")) {
        g_free(slant); slant = g_strdup("");
    }

    fontstring = g_strdup_printf("%s %s %s %s", name, weight, slant, size);
    gtk_font_button_set_font_name(w, fontstring);

    if (!g_ascii_strcasecmp(weight, "Bold")) rr_weight = RR_FONTWEIGHT_BOLD;
    if (!g_ascii_strcasecmp(slant, "Italic")) rr_slant = RR_FONTSLANT_ITALIC;
    if (!g_ascii_strcasecmp(slant, "Oblique")) rr_slant = RR_FONTSLANT_OBLIQUE;

    font = RrFontOpen(rrinst, name, atoi(size), rr_weight, rr_slant);
    g_free(fontstring);
    g_free(slant);
    g_free(weight);
    g_free(size);
    g_free(name);

    mapping = FALSE;

    return font;
}

static RrFont *write_font(GtkFontButton *w, const gchar *place)
{
    gchar *c;
    gchar *font, *node;
    const gchar *size = NULL;
    const gchar *bold = NULL;
    const gchar *italic = NULL;

    RrFontWeight weight = RR_FONTWEIGHT_NORMAL;
    RrFontSlant slant = RR_FONTSLANT_NORMAL;

    if (mapping) return;

    font = g_strdup(gtk_font_button_get_font_name(w));
    while ((c = strrchr(font, ' '))) {
        if (!bold && !italic && !size && atoi(c+1))
            size = c+1;
        else if (!bold && !italic && !g_ascii_strcasecmp(c+1, "italic"))
            italic = c+1;
        else if (!bold && !g_ascii_strcasecmp(c+1, "bold"))
            bold = c+1;
        else
            break;
        *c = '\0';
    }
    if (!bold) bold = "Normal";
    if (!italic) italic = "Normal";

    node = g_strdup_printf("theme/font:place=%s/name", place);
    tree_set_string(node, font);
    g_free(node);

    node = g_strdup_printf("theme/font:place=%s/size", place);
    tree_set_string(node, size);
    g_free(node);

    node = g_strdup_printf("theme/font:place=%s/weight", place);
    tree_set_string(node, bold);
    g_free(node);

    node = g_strdup_printf("theme/font:place=%s/slant", place);
    tree_set_string(node, italic);
    g_free(node);

    if (!g_ascii_strcasecmp(bold, "Bold")) weight = RR_FONTWEIGHT_BOLD;
    if (!g_ascii_strcasecmp(italic, "Italic")) slant = RR_FONTSLANT_ITALIC;
    if (!g_ascii_strcasecmp(italic, "Oblique")) slant = RR_FONTSLANT_OBLIQUE;

    return RrFontOpen(rrinst, font, atoi(size), weight, slant);

    g_free(font);

}
