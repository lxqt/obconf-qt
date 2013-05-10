/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   mouse.c for ObConf, the configuration tool for Openbox
   Copyright (c) 2007        Dana Jansens

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
#include <obt/xml.h>

static gboolean   mapping = FALSE;
static xmlNodePtr saved_custom = NULL;

#define TITLEBAR_MAXIMIZE 0
#define TITLEBAR_SHADE    1
#define TITLEBAR_CUSTOM   2

static gint read_doubleclick_action();
static void write_doubleclick_action(gint a);
static void on_titlebar_doubleclick_custom_activate(GtkMenuItem *w,
                                                    gpointer data);
static void enable_stuff();

void mouse_setup_tab()
{
    GtkWidget *w;
    gint a;

    mapping = TRUE;

    w = get_widget("focus_mouse");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w),
                                 tree_get_bool("focus/followMouse", FALSE));

    w = get_widget("focus_delay");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w),
                              tree_get_int("focus/focusDelay", 0));

    w = get_widget("focus_raise");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w),
                                 tree_get_bool("focus/raiseOnFocus", FALSE));

    w = get_widget("focus_notlast");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w),
                                 !tree_get_bool("focus/focusLast", TRUE));

    w = get_widget("focus_under_mouse");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w),
                                 tree_get_bool("focus/underMouse", FALSE));

    w = get_widget("doubleclick_time");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w),
                              tree_get_int("mouse/doubleClickTime", 200));


    w = get_widget("titlebar_doubleclick");
    a = read_doubleclick_action();
    if (a == TITLEBAR_CUSTOM) {
        GtkWidget *i = gtk_menu_item_new_with_label(_("Custom actions"));
        g_signal_connect(i, "activate",
                         G_CALLBACK (on_titlebar_doubleclick_custom_activate),
                         NULL);
        gtk_menu_shell_append
            (GTK_MENU_SHELL
             (gtk_option_menu_get_menu
              (GTK_OPTION_MENU(w))), i);
    }
    gtk_option_menu_set_history(GTK_OPTION_MENU(w), a);

    enable_stuff();

    mapping = FALSE;
}

static void enable_stuff()
{
    GtkWidget *w;
    gboolean b;

    w = get_widget("focus_mouse");
    b = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));

    w = get_widget("focus_delay");
    gtk_widget_set_sensitive(w, b);
    w = get_widget("focus_delay_label");
    gtk_widget_set_sensitive(w, b);
    w = get_widget("focus_delay_label_units");
    gtk_widget_set_sensitive(w, b);
    w = get_widget("focus_raise");
    gtk_widget_set_sensitive(w, b);
    w = get_widget("focus_notlast");
    gtk_widget_set_sensitive(w, b);
    w = get_widget("focus_under_mouse");
    gtk_widget_set_sensitive(w, b);
}

void on_focus_mouse_toggled(GtkToggleButton *w, gpointer data)
{
    gboolean b;

    if (mapping) return;

    b = gtk_toggle_button_get_active(w);
    tree_set_bool("focus/followMouse", b);

    enable_stuff();
}

void on_focus_delay_value_changed(GtkSpinButton *w, gpointer data)
{
    if (mapping) return;

    tree_set_int("focus/focusDelay", gtk_spin_button_get_value_as_int(w));
}

void on_focus_raise_toggled(GtkToggleButton *w, gpointer data)
{
    if (mapping) return;

    tree_set_bool("focus/raiseOnFocus", gtk_toggle_button_get_active(w));
}

void on_focus_notlast_toggled(GtkToggleButton *w, gpointer data)
{
    if (mapping) return;

    tree_set_bool("focus/focusLast", !gtk_toggle_button_get_active(w));
}

void on_focus_under_mouse_toggled(GtkToggleButton *w, gpointer data)
{
    if (mapping) return;

    tree_set_bool("focus/underMouse", gtk_toggle_button_get_active(w));
}

void on_titlebar_doubleclick_maximize_activate(GtkMenuItem *w, gpointer data)
{
    if (mapping) return;

    write_doubleclick_action(TITLEBAR_MAXIMIZE);
}

void on_titlebar_doubleclick_shade_activate(GtkMenuItem *w, gpointer data)
{
    if (mapping) return;

    write_doubleclick_action(TITLEBAR_SHADE);
}

static void on_titlebar_doubleclick_custom_activate(GtkMenuItem *w,
                                                    gpointer data)
{
    if (mapping) return;

    write_doubleclick_action(TITLEBAR_CUSTOM);
}

void on_doubleclick_time_value_changed(GtkSpinButton *w, gpointer data)
{
    if (mapping) return;

    tree_set_int("mouse/doubleClickTime",
                 gtk_spin_button_get_value_as_int(w));
}

static gint read_doubleclick_action()
{
    xmlNodePtr n, top, c;
    gint max = 0, shade = 0, other = 0;

    top = tree_get_node("mouse/context:name=Titlebar"
                        "/mousebind:button=Left:action=DoubleClick", NULL);
    n = top->children;

    /* save the current state */
    saved_custom = xmlCopyNode(top, 1);

    /* remove the namespace from all the nodes under saved_custom..
       without recursion! */
    c = saved_custom;
    while (c) {
        xmlSetNs(c, NULL);
        if (c->children)
            c = c->children;
        else if (c->next)
            c = c->next;
        while (c->parent && !c->parent->next)
            c = c->parent;
        if (!c->parent)
            c = NULL;
    }

    while (n) {
        if (!xmlStrcmp(n->name, (const xmlChar*)"action")) {
            if (obt_xml_attr_contains(n, "name", "ToggleMaximizeFull"))
                ++max;
            else if (obt_xml_attr_contains(n, "name", "ToggleShade"))
                ++shade;
            else
                ++other;
            
        }
        n = n->next;
    }

    if (max == 1 && shade == 0 && other == 0)
        return TITLEBAR_MAXIMIZE;
    if (max == 0 && shade == 1 && other == 0)
        return TITLEBAR_SHADE;

    return TITLEBAR_CUSTOM;
}

static void write_doubleclick_action(gint a)
{
    xmlNodePtr n;

    n = tree_get_node("mouse/context:name=Titlebar"
                      "/mousebind:button=Left:action=DoubleClick", NULL);

    /* remove all children */
    while (n->children) {
        xmlUnlinkNode(n->children);
        xmlFreeNode(n->children);
    }

    if (a == TITLEBAR_MAXIMIZE) {
        n = xmlNewChild(n, NULL, "action", NULL);
        xmlSetProp(n, "name", "ToggleMaximizeFull");
    } else if (a == TITLEBAR_SHADE) {
        n = xmlNewChild(n, NULL, "action", NULL);
        xmlSetProp(n, "name", "ToggleShade");
    } else {
        xmlNodePtr c = saved_custom->children;
        while (c) {
            xmlAddChild(n, xmlCopyNode(c, 1));
            c = c->next;
        }
    }

    tree_apply();
}
