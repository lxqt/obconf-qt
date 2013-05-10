/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   margins.c for ObConf, the configuration tool for Openbox
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

static gboolean mapping = FALSE;

void margins_setup_tab()
{
    GtkWidget *w, *w1, *w2, *w3;
    GtkSizeGroup *group;
    gchar *s;
    gint pos;

    mapping = TRUE;

    w = get_widget("margins_left");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w),
                              tree_get_int("margins/left", 0));

    w = get_widget("margins_right");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w),
                              tree_get_int("margins/right", 0));

    w = get_widget("margins_top");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w),
                              tree_get_int("margins/top", 0));

    w = get_widget("margins_bottom");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w),
                              tree_get_int("margins/bottom", 0));

    mapping = FALSE;
}

void on_margins_left_value_changed(GtkSpinButton *w, gpointer data)
{
    if (mapping) return;

    tree_set_int("margins/left", gtk_spin_button_get_value_as_int(w));
}

void on_margins_right_value_changed(GtkSpinButton *w, gpointer data)
{
    if (mapping) return;

    tree_set_int("margins/right", gtk_spin_button_get_value_as_int(w));
}

void on_margins_top_value_changed(GtkSpinButton *w, gpointer data)
{
    if (mapping) return;

    tree_set_int("margins/top", gtk_spin_button_get_value_as_int(w));
}

void on_margins_bottom_value_changed(GtkSpinButton *w, gpointer data)
{
    if (mapping) return;

    tree_set_int("margins/bottom", gtk_spin_button_get_value_as_int(w));
}

