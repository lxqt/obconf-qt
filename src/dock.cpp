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


#define POSITION_TOPLEFT     0
#define POSITION_TOP         1
#define POSITION_TOPRIGHT    2
#define POSITION_LEFT        3
#define POSITION_RIGHT       4
#define POSITION_BOTTOMLEFT  5
#define POSITION_BOTTOM      6
#define POSITION_BOTTOMRIGHT 7
#define POSITION_FLOATING    8

#define DIRECTION_VERTICAL   0
#define DIRECTION_HORIZONTAL 1

static void dock_enable_stuff();

void MainDialog::dock_setup_tab() {
  gchar* s;
  gint pos;

  s = tree_get_string("dock/position", "TopLeft");

  if(!strcasecmp(s, "Top"))              pos = POSITION_TOP;
  else if(!strcasecmp(s, "TopRight"))    pos = POSITION_TOPRIGHT;
  else if(!strcasecmp(s, "Left"))        pos = POSITION_LEFT;
  else if(!strcasecmp(s, "Right"))       pos = POSITION_RIGHT;
  else if(!strcasecmp(s, "BottomLeft"))  pos = POSITION_BOTTOMLEFT;
  else if(!strcasecmp(s, "Bottom"))      pos = POSITION_BOTTOM;
  else if(!strcasecmp(s, "BottomRight")) pos = POSITION_BOTTOMRIGHT;
  else if(!strcasecmp(s, "Floating"))    pos = POSITION_FLOATING;
  else                                    pos = POSITION_TOPLEFT;

  g_free(s);

#if 0
  // FIXME
  w = get_widget("dock_position");
  gtk_option_menu_set_history(GTK_OPTION_MENU(w), pos);

  w = get_widget("dock_float_x");
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(w),
                            tree_get_int("dock/floatingX", 0));

  w = get_widget("dock_float_y");
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(w),
                            tree_get_int("dock/floatingY", 0));

  s = tree_get_string("dock/stacking", "Above");

  if(!strcasecmp(s, "Normal"))
    w = get_widget("dock_stack_normal");
  else if(!strcasecmp(s, "Below"))
    w = get_widget("dock_stack_bottom");
  else
    w = get_widget("dock_stack_top");

  g_free(s);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), TRUE);

  w = get_widget("dock_direction");
  s = tree_get_string("dock/direction", "Vertical");

  if(!strcasecmp(s, "Horizontal")) pos = DIRECTION_HORIZONTAL;
  else                              pos = DIRECTION_VERTICAL;

  g_free(s);
  gtk_option_menu_set_history(GTK_OPTION_MENU(w), pos);

  w = get_widget("dock_nostrut");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w),
                               tree_get_bool("dock/noStrut", FALSE));

  w = get_widget("dock_hide");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w),
                               tree_get_bool("dock/autoHide", FALSE));

  w = get_widget("dock_hide_delay");
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(w),
                            tree_get_int("dock/hideDelay", 300));

  w = get_widget("dock_show_delay");
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(w),
                            tree_get_int("dock/showDelay", 300));

  dock_enable_stuff();

#endif
}

static void dock_enable_stuff() {
  /* FIXME
   *  GtkWidget* w, *s;
   *  gboolean b;
   *
   *  w = get_widget("dock_position");
   *  b = gtk_option_menu_get_history(GTK_OPTION_MENU(w)) == POSITION_FLOATING;
   *
   *  s = get_widget("dock_float_x");
   *  gtk_widget_set_sensitive(s, b);
   *  s = get_widget("dock_float_y");
   *  gtk_widget_set_sensitive(s, b);
   *  s = get_widget("dock_float_label");
   *  gtk_widget_set_sensitive(s, b);
   *  s = get_widget("dock_float_label_x");
   *  gtk_widget_set_sensitive(s, b);
   *  s = get_widget("dock_nostrut");
   *  gtk_widget_set_sensitive(s, !b);
   *
   *  w = get_widget("dock_hide");
   *  b = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
   *
   *  s = get_widget("dock_hide_delay");
   *  gtk_widget_set_sensitive(s, b);
   *  s = get_widget("dock_hide_label");
   *  gtk_widget_set_sensitive(s, b);
   *  s = get_widget("dock_hide_label_units");
   *  gtk_widget_set_sensitive(s, b);
   *  s = get_widget("dock_show_delay");
   *  gtk_widget_set_sensitive(s, b);
   *  s = get_widget("dock_show_label");
   *  gtk_widget_set_sensitive(s, b);
   *  s = get_widget("dock_show_label_units");
   *  gtk_widget_set_sensitive(s, b);
   */
}

#if 0
//FIXME
void MainDialog::on_dock_top_left_activate(GtkMenuItem* w, gpointer data) {
  tree_set_string("dock/position", "TopLeft");
  dock_enable_stuff();
}

void MainDialog::on_dock_top_activate(GtkMenuItem* w, gpointer data) {
  tree_set_string("dock/position", "Top");
  dock_enable_stuff();
}

void MainDialog::on_dock_top_right_activate(GtkMenuItem* w, gpointer data) {
  tree_set_string("dock/position", "TopRight");
  dock_enable_stuff();
}

void MainDialog::on_dock_left_activate(GtkMenuItem* w, gpointer data) {
  tree_set_string("dock/position", "Left");
  dock_enable_stuff();
}

void MainDialog::on_dock_right_activate(GtkMenuItem* w, gpointer data) {
  tree_set_string("dock/position", "Right");
  dock_enable_stuff();
}

void MainDialog::on_dock_bottom_left_activate(GtkMenuItem* w, gpointer data) {
  tree_set_string("dock/position", "BottomLeft");
  dock_enable_stuff();
}

void MainDialog::on_dock_bottom_activate(GtkMenuItem* w, gpointer data) {
  tree_set_string("dock/position", "Bottom");
  dock_enable_stuff();
}

void MainDialog::on_dock_bottom_right_activate(GtkMenuItem* w, gpointer data) {
  tree_set_string("dock/position", "BottomRight");
  dock_enable_stuff();
}

void MainDialog::on_dock_floating_activate(GtkMenuItem* w, gpointer data) {
  tree_set_string("dock/position", "Floating");
  dock_enable_stuff();
}

#endif

void MainDialog::on_dock_float_x_valueChanged(int newValue) {
  tree_set_int("dock/floatingX", newValue);
}

void MainDialog::on_dock_float_y_valueChanged(int newValue) {
  tree_set_int("dock/floatingY", newValue);
}

void MainDialog::on_dock_stacking_top_toggled(bool checked) {
  if(checked)
    tree_set_string("dock/stacking", "Above");
}

void MainDialog::on_dock_stacking_normal_toggled(bool checked) {
  if(checked)
    tree_set_string("dock/stacking", "Normal");
}

void MainDialog::on_dock_stacking_bottom_toggled(bool checked) {
  if(checked)
    tree_set_string("dock/stacking", "Below");
}

#if 0
// FIXME
void MainDialog::on_dock_horizontal_activate(GtkMenuItem* w, gpointer data) {
  tree_set_string("dock/direction", "Horizontal");
}

void MainDialog::on_dock_vertical_activate(GtkMenuItem* w, gpointer data) {
  tree_set_string("dock/direction", "Vertical");
}
#endif

void MainDialog::on_dock_nostrut_toggled(bool checked) {
  tree_set_bool("dock/noStrut", checked);
}

void MainDialog::on_dock_hide_toggled(bool checked) {
  tree_set_bool("dock/autoHide", checked);
  dock_enable_stuff();
}

void MainDialog::on_dock_hide_delay_valueChanged(int newValue) {
  tree_set_int("dock/hideDelay", newValue);
}

void MainDialog::on_dock_show_delay_valueChanged(int newValue) {
  tree_set_int("dock/showDelay", newValue);
}

