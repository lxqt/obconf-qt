/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   windows.c for ObConf, the configuration tool for Openbox
   Copyright (c) 2003-2008   Dana Jansens
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

static gboolean mapping = FALSE;

#define POPUP_NONPIXEL 0
#define POPUP_ALWAYS   1
#define POPUP_NEVER    2

#define POSITION_CENTER 0
#define POSITION_TOP    1
#define POSITION_FIXED  2

#define EDGE_CENTER 0
#define EDGE_LEFT   1
#define EDGE_RIGHT  2

static void enable_stuff();
static void write_fixed_position(const gchar *coord);

void moveresize_setup_tab()
{
    GtkWidget *w, *w1, *w2, *w3;
    GtkSizeGroup *group;
    gchar *s;
    gint pos, i;
    gboolean opp;

    mapping = TRUE;

    w1    = get_widget("resist_window");
    w2    = get_widget("resist_edge");
    w3    = get_widget("drag_threshold");
    group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
    gtk_size_group_add_widget(group, w1);
    gtk_size_group_add_widget(group, w2);
    gtk_size_group_add_widget(group, w3);

    w1    = get_widget("resist_window_label");
    w2    = get_widget("resist_edge_label");
    w3    = get_widget("drag_threshold_label");
    group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
    gtk_size_group_add_widget(group, w1);
    gtk_size_group_add_widget(group, w2);
    gtk_size_group_add_widget(group, w3);

    w = get_widget("resize_contents");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w),
                                 tree_get_bool("resize/drawContents", TRUE));

    w = get_widget("resist_window");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w),
                              tree_get_int("resistance/strength", 10));

    w = get_widget("resist_edge");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w),
                              tree_get_int("resistance/screen_edge_strength",
                                           20));

    w = get_widget("resize_popup");
    s = tree_get_string("resize/popupShow", "NonPixel");
    if (!strcasecmp(s, "Always"))     pos = POPUP_ALWAYS;
    else if (!strcasecmp(s, "Never")) pos = POPUP_NEVER;
    else                              pos = POPUP_NONPIXEL;
    g_free(s);
    gtk_option_menu_set_history(GTK_OPTION_MENU(w), pos);

    w = get_widget("drag_threshold");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w),
                              tree_get_int("mouse/dragThreshold", 8));

    w = get_widget("resize_position");
    s = tree_get_string("resize/popupPosition", "Center");
    if (!strcasecmp(s, "Top"))   pos = POSITION_TOP;
    if (!strcasecmp(s, "Fixed")) pos = POSITION_FIXED;
    else                         pos = POSITION_CENTER;
    g_free(s);
    gtk_option_menu_set_history(GTK_OPTION_MENU(w), pos);

    w = get_widget("fixed_x_popup");
    s = tree_get_string("resize/popupFixedPosition/x", "0");
    opp = s[0] == '-';
    if (s[0] == '-' || s[0] == '+') ++s;
    if (!strcasecmp(s, "Center")) pos = EDGE_CENTER;
    else if (opp) pos = EDGE_RIGHT;
    else pos = EDGE_LEFT;
    g_free(s);
    gtk_option_menu_set_history(GTK_OPTION_MENU(w), pos);

    w = get_widget("fixed_x_pos");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), MAX(atoi(s), 0));

    w = get_widget("fixed_y_popup");
    s = tree_get_string("resize/popupFixedPosition/y", "0");
    opp = s[0] == '-';
    if (!strcasecmp(s, "Center")) pos = EDGE_CENTER;
    else if (opp) pos = EDGE_RIGHT;
    else pos = EDGE_LEFT;
    g_free(s);
    gtk_option_menu_set_history(GTK_OPTION_MENU(w), pos);

    w = get_widget("fixed_y_pos");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), MAX(atoi(s), 0));

    i = tree_get_int("mouse/screenEdgeWarpTime", 400);

    w = get_widget("warp_edge");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), i != 0);

    w = get_widget("warp_edge_time");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), i ? i : 400);

    enable_stuff();

    mapping = FALSE;
}

static void enable_stuff()
{
    GtkWidget *w;
    gboolean b;

    w = get_widget("resize_popup");
    b = gtk_option_menu_get_history(GTK_OPTION_MENU(w)) != POPUP_NEVER;
    w = get_widget("resize_position");
    gtk_widget_set_sensitive(w, b);

    w = get_widget("warp_edge");
    b = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
    w = get_widget("warp_edge_time");
    gtk_widget_set_sensitive(w, b);

    w = get_widget("resize_position");
    b = gtk_option_menu_get_history(GTK_OPTION_MENU(w)) == POSITION_FIXED;
    w = get_widget("fixed_x_popup");
    gtk_widget_set_sensitive(w, b);
    w = get_widget("fixed_y_popup");
    gtk_widget_set_sensitive(w, b);
    if (!b) {
        w = get_widget("fixed_x_pos");
        gtk_widget_set_sensitive(w, FALSE);
        w = get_widget("fixed_y_pos");
        gtk_widget_set_sensitive(w, FALSE);
    }
    else {
        w = get_widget("fixed_x_popup");
        b = gtk_option_menu_get_history(GTK_OPTION_MENU(w)) != EDGE_CENTER;
        w = get_widget("fixed_x_pos");
        gtk_widget_set_sensitive(w, b);

        w = get_widget("fixed_y_popup");
        b = gtk_option_menu_get_history(GTK_OPTION_MENU(w)) != EDGE_CENTER;
        w = get_widget("fixed_y_pos");
        gtk_widget_set_sensitive(w, b);
    }
}

void on_resist_window_value_changed(GtkSpinButton *w, gpointer data)
{
    if (mapping) return;

    tree_set_int("resistance/strength", gtk_spin_button_get_value_as_int(w));
}

void on_resist_edge_value_changed(GtkSpinButton *w, gpointer data)
{
    if (mapping) return;

    tree_set_int("resistance/screen_edge_strength",
                 gtk_spin_button_get_value_as_int(w));
}

void on_resize_contents_toggled(GtkToggleButton *w, gpointer data)
{
    if (mapping) return;

    tree_set_bool("resize/drawContents", gtk_toggle_button_get_active(w));
}

void on_resize_popup_nonpixel_activate(GtkMenuItem *w, gpointer data)
{
    if (mapping) return;

    tree_set_string("resize/popupShow", "NonPixel");
    enable_stuff();
}

void on_resize_popup_always_activate(GtkMenuItem *w, gpointer data)
{
    if (mapping) return;

    tree_set_string("resize/popupShow", "Always");
    enable_stuff();
}

void on_resize_popup_never_activate(GtkMenuItem *w, gpointer data)
{
    if (mapping) return;

    tree_set_string("resize/popupShow", "Never");
    enable_stuff();
}

void on_drag_threshold_value_changed(GtkSpinButton *w, gpointer data)
{
    if (mapping) return;

    tree_set_int("mouse/dragThreshold",
                 gtk_spin_button_get_value_as_int(w));
}

void on_resize_position_center_activate(GtkMenuItem *w, gpointer data)
{
    if (mapping) return;

    tree_set_string("resize/popupPosition", "Center");
    enable_stuff();
}

void on_resize_position_top_activate(GtkMenuItem *w, gpointer data)
{
    if (mapping) return;

    tree_set_string("resize/popupPosition", "Top");
    enable_stuff();
}

void on_resize_position_fixed_activate(GtkMenuItem *w, gpointer data)
{
    if (mapping) return;

    tree_set_string("resize/popupPosition", "Fixed");
    enable_stuff();
}

static void write_fixed_position(const gchar *coord)
{
    GtkWidget *popup;
    gchar *popupname;
    gchar *val;
    gchar *valname;
    gint edge;

    g_assert(!strcmp(coord, "x") || !strcmp(coord, "y"));

    popupname = g_strdup_printf("fixed_%s_popup", coord);
    popup = get_widget(popupname);
    g_free(popupname);

    edge = gtk_option_menu_get_history(GTK_OPTION_MENU(popup));
    g_assert(edge == EDGE_CENTER || edge == EDGE_LEFT || edge == EDGE_RIGHT);

    if (edge == EDGE_CENTER)
        val = g_strdup("center");
    else {
        GtkWidget *spin;
        gchar *spinname;
        gint i;

        spinname = g_strdup_printf("fixed_%s_pos", coord);
        spin = get_widget(spinname);
        g_free(spinname);

        i = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));

        if (edge == EDGE_LEFT)
            val = g_strdup_printf("%d", i);
        else
            val = g_strdup_printf("-%d", i);
    }

    valname = g_strdup_printf("resize/popupFixedPosition/%s", coord);
    tree_set_string(valname, val);
    g_free(valname);
    g_free(val);
}


void on_fixed_x_position_left_activate(GtkMenuItem *w, gpointer data)
{
    if (mapping) return;

    write_fixed_position("x");
    enable_stuff();
}

void on_fixed_x_position_right_activate(GtkMenuItem *w, gpointer data)
{
    if (mapping) return;

    write_fixed_position("x");
    enable_stuff();
}

void on_fixed_x_position_center_activate(GtkMenuItem *w, gpointer data)
{
    if (mapping) return;

    write_fixed_position("x");
    enable_stuff();
}

void on_fixed_y_position_top_activate(GtkMenuItem *w, gpointer data)
{
    if (mapping) return;

    write_fixed_position("y");
    enable_stuff();
}

void on_fixed_y_position_bottom_activate(GtkMenuItem *w, gpointer data)
{
    if (mapping) return;

    write_fixed_position("y");
    enable_stuff();
}

void on_fixed_y_position_center_activate(GtkMenuItem *w, gpointer data)
{
    if (mapping) return;

    write_fixed_position("y");
    enable_stuff();
}

void on_fixed_x_pos_value_changed(GtkSpinButton *w, gpointer data)
{
    if (mapping) return;

    write_fixed_position("x");
}

void on_fixed_y_pos_value_changed(GtkSpinButton *w, gpointer data)
{
    if (mapping) return;

    write_fixed_position("y");
}

void on_warp_edge_toggled(GtkToggleButton *w, gpointer data)
{
    if (mapping) return;

    if (gtk_toggle_button_get_active(w)) {
        GtkWidget *w2;

        w2 = get_widget("warp_edge_time");
        tree_set_int("mouse/screenEdgeWarpTime",
                     gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(w2)));
    }
    else
        tree_set_int("mouse/screenEdgeWarpTime", 0);
    enable_stuff();
}

void on_warp_edge_time_value_changed(GtkSpinButton *w, gpointer data)
{
    if (mapping) return;

    tree_set_int("mouse/screenEdgeWarpTime",
                 gtk_spin_button_get_value_as_int(w));
}

