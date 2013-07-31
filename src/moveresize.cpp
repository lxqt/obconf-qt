/*
    Copyright (C) 2013  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>

    Part of the code in this file is taken from obconf:
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

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "maindialog.h"
#include <obrender/render.h>
#include "tree.h"

#include <QX11Info>
// FIXME: how to support XCB or Wayland?
#include <X11/Xlib.h>

using namespace Obconf;

extern RrInstance* rrinst; // defined in obconf-qt.cpp


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
static void write_fixed_position(const gchar* coord);

void MainDialog::moveresize_setup_tab() {
  gchar* s;
  gint pos, i;
  gboolean opp;

  ui.resize_contents->setChecked(tree_get_bool("resize/drawContents", TRUE));

  ui.resist_window->setValue(tree_get_int("resistance/strength", 10));

  ui.resist_edge->setValue(tree_get_int("resistance/screen_edge_strength", 20));

  s = tree_get_string("resize/popupShow", "NonPixel");

  if(!strcasecmp(s, "Always"))     pos = POPUP_ALWAYS;
  else if(!strcasecmp(s, "Never")) pos = POPUP_NEVER;
  else                              pos = POPUP_NONPIXEL;

  g_free(s);

  ui.resize_popup->setCurrentIndex(pos);

  ui.drag_threshold->setValue(tree_get_int("mouse/dragThreshold", 8));

  s = tree_get_string("resize/popupPosition", "Center");

  if(!strcasecmp(s, "Top"))   pos = POSITION_TOP;

  if(!strcasecmp(s, "Fixed")) pos = POSITION_FIXED;
  else                         pos = POSITION_CENTER;

  g_free(s);
  ui.resize_position->setCurrentIndex(pos);


  s = tree_get_string("resize/popupFixedPosition/x", "0");
  opp = s[0] == '-';

  if(s[0] == '-' || s[0] == '+')
    ++s;

  if(!strcasecmp(s, "Center")) pos = EDGE_CENTER;
  else if(opp) pos = EDGE_RIGHT;
  else pos = EDGE_LEFT;

  g_free(s);

  ui.fixed_x_popup->setCurrentIndex(pos);
  ui.fixed_x_pos->setValue(MAX(atoi(s), 0));

  s = tree_get_string("resize/popupFixedPosition/y", "0");
  opp = s[0] == '-';

  if(!strcasecmp(s, "Center")) pos = EDGE_CENTER;
  else if(opp) pos = EDGE_RIGHT;
  else pos = EDGE_LEFT;

  g_free(s);

  ui.fixed_y_popup->setCurrentIndex(pos);
  ui.fixed_y_pos->setValue(MAX(atoi(s), 0));

  i = tree_get_int("mouse/screenEdgeWarpTime", 400);

  ui.warp_edge->setChecked(i != 0);
  ui.warp_edge_time->setValue(i ? i : 400);

  // FIXME enable_stuff();
}

#if 0
static void enable_stuff() {
  GtkWidget* w;
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

  if(!b) {
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

#endif

void MainDialog::on_resist_window_valueChanged(int newValue) {
  tree_set_int("resistance/strength", newValue);
}

void MainDialog::on_resist_edge_valueChanged(int newValue) {
  tree_set_int("resistance/screen_edge_strength", newValue);
}

void MainDialog::on_resize_contents_toggled(bool checked) {
  tree_set_bool("resize/drawContents", checked);
}

void MainDialog::on_resize_popup_currentIndexChanged(int index) {
  switch(index) {
    case POPUP_NONPIXEL:
      tree_set_string("resize/popupShow", "NonPixel");
      break;
    case POPUP_ALWAYS:
      tree_set_string("resize/popupShow", "Always");
      break;
    case POPUP_NEVER:
      tree_set_string("resize/popupShow", "Never");
      break;
  }

  // FIXME enable_stuff();
}

void MainDialog::on_drag_threshold_valueChanged(int newValue) {
  tree_set_int("mouse/dragThreshold", newValue);
}

/*
 v *oid MainDialog::on_resize_position_center_activate(GtkMenuItem* w, gpointer data) {
   tree_set_string("resize/popupPosition", "Center");
   enable_stuff();
 }

 void MainDialog::on_resize_position_top_activate(GtkMenuItem* w, gpointer data) {
   tree_set_string("resize/popupPosition", "Top");
   enable_stuff();
 }

 void MainDialog::on_resize_position_fixed_activate(GtkMenuItem* w, gpointer data) {
   tree_set_string("resize/popupPosition", "Fixed");
   enable_stuff();
 }

 */

static void write_fixed_position(const gchar* coord) {
#if 0
  GtkWidget* popup;
  gchar* popupname;
  gchar* val;
  gchar* valname;
  gint edge;

  g_assert(!strcmp(coord, "x") || !strcmp(coord, "y"));

  popupname = g_strdup_printf("fixed_%s_popup", coord);
  popup = get_widget(popupname);
  g_free(popupname);

  edge = gtk_option_menu_get_history(GTK_OPTION_MENU(popup));
  g_assert(edge == EDGE_CENTER || edge == EDGE_LEFT || edge == EDGE_RIGHT);

  if(edge == EDGE_CENTER)
    val = g_strdup("center");
  else {
    GtkWidget* spin;
    gchar* spinname;
    gint i;

    spinname = g_strdup_printf("fixed_%s_pos", coord);
    spin = get_widget(spinname);
    g_free(spinname);

    i = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));

    if(edge == EDGE_LEFT)
      val = g_strdup_printf("%d", i);
    else
      val = g_strdup_printf("-%d", i);
  }

  valname = g_strdup_printf("resize/popupFixedPosition/%s", coord);
  tree_set_string(valname, val);
  g_free(valname);
  g_free(val);
#endif
}

/*
 v *oid MainDialog::on_fixed_x_position_left_activate(GtkMenuItem* w, gpointer data) {


   write_fixed_position("x");
   enable_stuff();
 }

 void MainDialog::on_fixed_x_position_right_activate(GtkMenuItem* w, gpointer data) {


   write_fixed_position("x");
   enable_stuff();
 }

 void MainDialog::on_fixed_x_position_center_activate(GtkMenuItem* w, gpointer data) {


   write_fixed_position("x");
   enable_stuff();
 }

 void MainDialog::on_fixed_y_position_top_activate(GtkMenuItem* w, gpointer data) {


   write_fixed_position("y");
   enable_stuff();
 }

 void MainDialog::on_fixed_y_position_bottom_activate(GtkMenuItem* w, gpointer data) {


   write_fixed_position("y");
   enable_stuff();
 }

 void MainDialog::on_fixed_y_position_center_activate(GtkMenuItem* w, gpointer data) {


   write_fixed_position("y");
   enable_stuff();
 }

 */

void MainDialog::on_fixed_x_pos_valueChanged(int newValue) {
  write_fixed_position("x");
}

void MainDialog::on_fixed_y_pos_valueChanged(int newValue) {
  write_fixed_position("y");
}

void MainDialog::on_warp_edge_toggled(bool checked) {
  if(checked) {
    tree_set_int("mouse/screenEdgeWarpTime", ui.warp_edge_time->value());
  }
  else
    tree_set_int("mouse/screenEdgeWarpTime", 0);

  // FIXME enable_stuff();
}

void MainDialog::on_warp_edge_time_valueChanged(int newValue) {
  tree_set_int("mouse/screenEdgeWarpTime", newValue);
}
