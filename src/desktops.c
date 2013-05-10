/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   desktops.c for ObConf, the configuration tool for Openbox
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
#include "gettext.h"

#include <gdk/gdkx.h>

static gboolean mapping = FALSE;

static GtkListStore *desktop_store;
static int num_desktops;
static GList *desktop_names;

static void desktops_read_names();
static void desktops_write_names();
static void desktops_write_number();
static void on_desktop_names_cell_edited(GtkCellRendererText *cell,
                                         const gchar *path_string,
                                         const gchar *new_text,
                                         gpointer data);
static void enable_stuff();

void desktops_setup_tab()
{
    GtkWidget *w;
    GtkCellRenderer *render;
    GtkTreeViewColumn *column;
    gint i;

    mapping = TRUE;

    w = get_widget("desktop_num");
    num_desktops = tree_get_int("desktops/number", 4);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), num_desktops);

    w = get_widget("desktop_names");
    desktop_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_BOOLEAN);
    gtk_tree_view_set_model(GTK_TREE_VIEW(w), GTK_TREE_MODEL(desktop_store));
    g_object_unref (desktop_store);

    gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(w)),
                                GTK_SELECTION_SINGLE);

    render = gtk_cell_renderer_text_new();
    g_signal_connect(render, "edited",
                     G_CALLBACK (on_desktop_names_cell_edited),
                     NULL);

    column = gtk_tree_view_column_new_with_attributes
        ("Name", render, "text", 0, "editable", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(w), column);

    desktops_read_names();

    i = tree_get_int("desktops/popupTime", 875);

    w = get_widget("desktop_popup");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), i != 0);

    w = get_widget("desktop_popup_time");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), i ? i : 875);

    enable_stuff();

    mapping = FALSE;
}

static void enable_stuff()
{
    GtkWidget *w;
    gboolean b;

    w = get_widget("desktop_popup");
    b = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
    w = get_widget("desktop_popup_time");
    gtk_widget_set_sensitive(w, b);
}

void on_desktop_num_value_changed(GtkSpinButton *w, gpointer data)
{
    if (mapping) return;

    num_desktops = gtk_spin_button_get_value(w);

    desktops_write_number();
    desktops_read_names();
}

static void on_desktop_names_cell_edited(GtkCellRendererText *cell,
                                         const gchar *path_string,
                                         const gchar *new_text,
                                         gpointer data)
{
    GtkTreePath *path;
    GtkTreeIter it;
    gchar *old_text;
    GList *lit;
    gint i;

    if (mapping) return;

    path = gtk_tree_path_new_from_string (path_string);
    gtk_tree_model_get_iter(GTK_TREE_MODEL(desktop_store), &it, path);

    gtk_tree_model_get(GTK_TREE_MODEL(desktop_store), &it, 0, &old_text, -1);
    g_free(old_text);

    i = gtk_tree_path_get_indices(path)[0];
    lit = g_list_nth(desktop_names, i);

    g_free(lit->data);
    lit->data = g_strdup(new_text);
    if (new_text[0]) /* not empty */
        gtk_list_store_set(desktop_store, &it, 0, lit->data, -1);
    else
        gtk_list_store_set(desktop_store, &it, 0, _("(Unnamed desktop)"), -1);

    desktops_write_names();
}

static void desktops_read_names()
{
    GtkTreeIter it;
    xmlNodePtr n;
    gint i;
    GList *lit;

    gtk_list_store_clear(desktop_store);

    for (lit = desktop_names; lit; lit = g_list_next(lit))
        g_free(lit->data);
    g_list_free(desktop_names);
    desktop_names = NULL;

    i = 0;
    n = tree_get_node("desktops/names", NULL)->children;
    while (n) {
        gchar *name;

        if (!xmlStrcmp(n->name, (const xmlChar*)"name")) {
            name = obt_xml_node_string(n);

            desktop_names = g_list_append(desktop_names, name);

            gtk_list_store_append(desktop_store, &it);
            gtk_list_store_set(desktop_store, &it,
                               0, (name[0] ? name : _("(Unnamed desktop)")),
                               1, TRUE,
                               -1);
            ++i;
        }

        n = n->next;
    }

    while (i < num_desktops) {
        gchar *name = g_strdup("");

        desktop_names = g_list_append(desktop_names, name);

        gtk_list_store_append(desktop_store, &it);
        gtk_list_store_set(desktop_store, &it,
                           0, _("(Unnamed desktop)"),
                           1, TRUE,
                           -1);
        ++i;
    }
}

static void desktops_write_names()
{
    gchar **s;
    GList *lit;
    xmlNodePtr n, c;
    gint num = 0, last = -1;

    n = tree_get_node("desktops/names", NULL);
    while ((c = n->children)) {
        xmlUnlinkNode(c);
        xmlFreeNode(c);
    }

    for (lit = desktop_names; lit; lit = g_list_next(lit)) {
        if (((gchar*)lit->data)[0]) /* not empty */
            last = num;
        ++num;
    }

    num = 0;
    for (lit = desktop_names; lit && num <= last; lit = g_list_next(lit)) {
        xmlNewTextChild(n, NULL, "name", lit->data);
        ++num;
    }
    tree_apply();

    /* make openbox re-set the property */
    XDeleteProperty(GDK_DISPLAY(), GDK_ROOT_WINDOW(),
                    gdk_x11_get_xatom_by_name("_NET_DESKTOP_NAMES"));
}

static void desktops_write_number()
{
    XEvent ce;

    tree_set_int("desktops/number", num_desktops);

    ce.xclient.type = ClientMessage;
    ce.xclient.message_type =
        gdk_x11_get_xatom_by_name("_NET_NUMBER_OF_DESKTOPS");
    ce.xclient.display = GDK_DISPLAY();
    ce.xclient.window = GDK_ROOT_WINDOW();
    ce.xclient.format = 32;
    ce.xclient.data.l[0] = num_desktops;
    ce.xclient.data.l[1] = 0;
    ce.xclient.data.l[2] = 0;
    ce.xclient.data.l[3] = 0;
    ce.xclient.data.l[4] = 0;
    XSendEvent(GDK_DISPLAY(), GDK_ROOT_WINDOW(), FALSE,
               SubstructureNotifyMask | SubstructureRedirectMask,
               &ce);
}

void on_desktop_popup_toggled(GtkToggleButton *w, gpointer data)
{
    if (mapping) return;

    if (gtk_toggle_button_get_active(w)) {
        GtkWidget *w2;

        w2 = get_widget("desktop_popup_time");
        tree_set_int("desktops/popupTime",
                     gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(w2)));
    }
    else
        tree_set_int("desktops/popupTime", 0);
    enable_stuff();
}

void on_desktop_popup_time_value_changed(GtkSpinButton *w, gpointer data)
{
    if (mapping) return;

    tree_set_int("desktops/popupTime", gtk_spin_button_get_value_as_int(w));
}
