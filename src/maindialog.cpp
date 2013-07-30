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

#include <string.h>
#include <math.h>
#include <alloca.h>
#include <stdlib.h>
#include "tree.h"

#include <QX11Info>

#include <QSettings>
#include <QDir>
#include <QFile>
#include <QStringBuilder>
#include <QFont>

#include <obt/xml.h>
#include <obrender/render.h>
#include "archive.h"
#include "theme.h"

// FIXME: how to support XCB or Wayland?
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

extern RrInstance* rrinst; // defined in obconf-qt.cpp

using namespace Obconf;

MainDialog::MainDialog():
  QDialog() {

  ui.setupUi(this);

  /* read the config flie */
  loadSettings();

  theme_setup_tab();
  appearance_setup_tab();
  windows_setup_tab();
  mouse_setup_tab();
  moveresize_setup_tab();
  margins_setup_tab();
  desktops_setup_tab();
  dock_setup_tab();

  /* connect signals and slots */
  QMetaObject::connectSlotsByName(this);
}

MainDialog::~MainDialog() {

}

void MainDialog::loadSettings() {

}

void MainDialog::accept() {
  QDialog::accept();
}

void MainDialog::reject() {
  /* restore to original settings */

  QDialog::reject();
}

// ----------------------- windows -------------------------------

#define PLACE_ON_FIXED 0
#define PLACE_ON_PRIMARY 0
#define PLACE_ON_ACTIVE 1
#define PLACE_ON_MOUSE 2
#define PLACE_ON_ALL 3

void MainDialog::windows_setup_tab() {
  gchar* s;
  ui.focus_new->setChecked(tree_get_bool("focus/focusNew", TRUE));

  s = tree_get_string("placement/policy", "Smart");
  ui.place_mouse->setChecked(!g_ascii_strcasecmp(s, "UnderMouse"));
  g_free(s);

  int index;
  s = tree_get_string("placement/monitor", "Any");
  if(!g_ascii_strcasecmp(s, "Active"))
    index = PLACE_ON_ACTIVE;
  else if(!g_ascii_strcasecmp(s, "Mouse"))
    index = PLACE_ON_MOUSE;
  else if(!g_ascii_strcasecmp(s, "Primary"))
    index = PLACE_ON_PRIMARY;
  else
    index = PLACE_ON_ALL;
  g_free(s);
  ui.place_active_popup->setCurrentIndex(index);

  s = tree_get_string("placement/primaryMonitor", "");
  if(!g_ascii_strcasecmp(s, "Active"))
    index = PLACE_ON_ACTIVE;
  else if(!g_ascii_strcasecmp(s, "Mouse"))
    index = PLACE_ON_MOUSE;
  else {
    index = PLACE_ON_FIXED;
    ui.fixed_monitor->setValue(tree_get_int("placement/primaryMonitor", 1));
  }
  g_free(s);
  ui.primary_monitor_popup->setCurrentIndex(index);

  // FIXME enable_stuff();
}

static void enable_stuff() {
#if 0
  GtkWidget* w;
  gboolean b;

  w = get_widget("place_mouse");
  b = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));

  w = get_widget("primary_monitor_popup");
  b = gtk_option_menu_get_history(GTK_OPTION_MENU(w)) == PLACE_ON_FIXED;
  w = get_widget("fixed_monitor");
  gtk_widget_set_sensitive(w, b);
#endif
}

#if 0

void MainDialog::on_primary_monitor_active_activate() {

  tree_set_string("placement/primaryMonitor", "Active");
  enable_stuff();
}

void MainDialog::on_primary_monitor_mouse_activate() {
  tree_set_string("placement/primaryMonitor", "Mouse");
  enable_stuff();
}

void MainDialog::on_primary_monitor_fixed_activate() {
  GtkWidget* w2;
  w2 = get_widget("fixed_monitor");
  tree_set_int("placement/primaryMonitor",
               gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(w2)));
  enable_stuff();
}

void MainDialog::on_fixed_monitor_valueChanged(int newValue) {


  tree_set_int("placement/primaryMonitor",
               newValue);
}

void MainDialog::on_focus_new_toggled(bool checked) {


  tree_set_bool("focus/focusNew", checked);
}

void MainDialog::on_place_mouse_toggled(bool checked) {


  tree_set_string("placement/policy",
                  (checked ?
                   "UnderMouse" : "Smart"));
  enable_stuff();
}

void MainDialog::on_place_active_popup_all_activate() {
  tree_set_string("placement/monitor", "Any");
}

void MainDialog::on_place_active_popup_active_activate() {
  tree_set_string("placement/monitor", "Active");
}

void MainDialog::on_place_active_popup_mouse_activate() {
  tree_set_string("placement/monitor", "Mouse");
}

void MainDialog::on_place_active_popup_primary_activate() {
  tree_set_string("placement/monitor", "Primary");
}
#endif


// --------------------Move Resize------------------------



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

#if 0
  w = get_widget("fixed_x_popup");
  s = tree_get_string("resize/popupFixedPosition/x", "0");
  opp = s[0] == '-';

  if(s[0] == '-' || s[0] == '+') ++s;

  if(!strcasecmp(s, "Center")) pos = EDGE_CENTER;
  else if(opp) pos = EDGE_RIGHT;
  else pos = EDGE_LEFT;

  g_free(s);
  gtk_option_menu_set_history(GTK_OPTION_MENU(w), pos);

  w = get_widget("fixed_x_pos");
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), MAX(atoi(s), 0));

  w = get_widget("fixed_y_popup");
  s = tree_get_string("resize/popupFixedPosition/y", "0");
  opp = s[0] == '-';

  if(!strcasecmp(s, "Center")) pos = EDGE_CENTER;
  else if(opp) pos = EDGE_RIGHT;
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
#endif
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

/*
void MainDialog::on_resize_popup_nonpixel_activate(GtkMenuItem* w, gpointer data) {
  tree_set_string("resize/popupShow", "NonPixel");
  enable_stuff();
}

void MainDialog::on_resize_popup_always_activate(GtkMenuItem* w, gpointer data) {
  tree_set_string("resize/popupShow", "Always");
  enable_stuff();
}

void MainDialog::on_resize_popup_never_activate(GtkMenuItem* w, gpointer data) {


  tree_set_string("resize/popupShow", "Never");
  enable_stuff();
}
*/

void MainDialog::on_drag_threshold_valueChanged(int newValue) {
  tree_set_int("mouse/dragThreshold", newValue);
}

/*
void MainDialog::on_resize_position_center_activate(GtkMenuItem* w, gpointer data) {
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
void MainDialog::on_fixed_x_position_left_activate(GtkMenuItem* w, gpointer data) {


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

  enable_stuff();
}

void MainDialog::on_warp_edge_time_valueChanged(int newValue) {
  tree_set_int("mouse/screenEdgeWarpTime", newValue);
}

//------------------------ Margins --------------------------------

void MainDialog::margins_setup_tab() {
  ui.margins_left->setValue(tree_get_int("margins/left", 0));
  ui.margins_right->setValue(tree_get_int("margins/right", 0));
  ui.margins_top->setValue(tree_get_int("margins/top", 0));
  ui.margins_bottom->setValue(tree_get_int("margins/bottom", 0));
}

void MainDialog::on_margins_left_valueChanged(int newValue) {
  tree_set_int("margins/left", newValue);
}

void MainDialog::on_margins_right_valueChanged(int newValue) {
  tree_set_int("margins/right", newValue);
}

void MainDialog::on_margins_top_valueChanged(int newValue) {
  tree_set_int("margins/top", newValue);
}

void MainDialog::on_margins_bottom_valueChanged(int newValue) {
  tree_set_int("margins/bottom", newValue);
}


// ---------------------- mouse -----------------------------------

static gboolean   mapping = FALSE;
static xmlNodePtr saved_custom = NULL;

#define TITLEBAR_MAXIMIZE 0
#define TITLEBAR_SHADE    1
#define TITLEBAR_CUSTOM   2

static gint read_doubleclick_action();
static void write_doubleclick_action(gint a);
//static void MainDialog::on_titlebar_doubleclick_custom_activate(GtkMenuItem* w,
//    gpointer data);
static void enable_stuff();

void MainDialog::mouse_setup_tab() {
  gint a;

  ui.focus_mouse->setChecked(tree_get_bool("focus/followMouse", FALSE));
  ui.focus_delay->setValue(tree_get_int("focus/focusDelay", 0));
  ui.focus_raise->setChecked(tree_get_bool("focus/raiseOnFocus", FALSE));
  ui.focus_notlast->setChecked(!tree_get_bool("focus/focusLast", TRUE));
  ui.focus_under_mouse->setChecked(tree_get_bool("focus/underMouse", FALSE));
  ui.doubleclick_time->setValue(tree_get_int("mouse/doubleClickTime", 200));

  // w = get_widget("titlebar_doubleclick");
  a = read_doubleclick_action();

  if(a == TITLEBAR_CUSTOM) {
    ui.titlebar_doubleclick->addItem(tr("Custom actions"));
/*
    GtkWidget* i = gtk_menu_item_new_with_label(_());
    g_signal_connect(i, "activate",
                     G_CALLBACK(on_titlebar_doubleclick_custom_activate),
                     NULL);
    gtk_menu_shell_append
    (GTK_MENU_SHELL
     (gtk_option_menu_get_menu
      (GTK_OPTION_MENU(w))), i);
*/
  }
  ui.titlebar_doubleclick->setCurrentIndex(a);
  // FIXME enable_stuff();
}

/*
static void enable_stuff() {
  GtkWidget* w;
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
*/

void MainDialog::on_focus_mouse_toggled(bool checked) {
  tree_set_bool("focus/followMouse", checked);
  enable_stuff();
}

void MainDialog::on_focus_delay_valueChanged(int newValue) {
  tree_set_int("focus/focusDelay", newValue);
}

void MainDialog::on_focus_raise_toggled(bool checked) {
  tree_set_bool("focus/raiseOnFocus", checked);
}

void MainDialog::on_focus_notlast_toggled(bool checked) {
  tree_set_bool("focus/focusLast", !checked);
}

void MainDialog::on_focus_under_mouse_toggled(bool checked) {
  tree_set_bool("focus/underMouse", checked);
}

/*
void MainDialog::on_titlebar_doubleclick_maximize_activate(GtkMenuItem* w, gpointer data) {
  write_doubleclick_action(TITLEBAR_MAXIMIZE);
}

void MainDialog::on_titlebar_doubleclick_shade_activate(GtkMenuItem* w, gpointer data) {


  write_doubleclick_action(TITLEBAR_SHADE);
}

static void MainDialog::on_titlebar_doubleclick_custom_activate(GtkMenuItem* w,
    gpointer data) {


  write_doubleclick_action(TITLEBAR_CUSTOM);
}
*/

void MainDialog::on_doubleclick_time_valueChanged(int newValue) {
  tree_set_int("mouse/doubleClickTime", newValue);
}

static gint read_doubleclick_action() {
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

  while(c) {
    xmlSetNs(c, NULL);

    if(c->children)
      c = c->children;
    else if(c->next)
      c = c->next;

    while(c->parent && !c->parent->next)
      c = c->parent;

    if(!c->parent)
      c = NULL;
  }

  while(n) {
    if(!xmlStrcmp(n->name, (const xmlChar*)"action")) {
      if(obt_xml_attr_contains(n, "name", "ToggleMaximizeFull"))
        ++max;
      else if(obt_xml_attr_contains(n, "name", "ToggleShade"))
        ++shade;
      else
        ++other;

    }

    n = n->next;
  }

  if(max == 1 && shade == 0 && other == 0)
    return TITLEBAR_MAXIMIZE;

  if(max == 0 && shade == 1 && other == 0)
    return TITLEBAR_SHADE;

  return TITLEBAR_CUSTOM;
}

static void write_doubleclick_action(gint a) {
  xmlNodePtr n;

  n = tree_get_node("mouse/context:name=Titlebar"
                    "/mousebind:button=Left:action=DoubleClick", NULL);

  /* remove all children */
  while(n->children) {
    xmlUnlinkNode(n->children);
    xmlFreeNode(n->children);
  }

  if(a == TITLEBAR_MAXIMIZE) {
    n = xmlNewChild(n, NULL, (xmlChar*)"action", NULL);
    xmlSetProp(n, (xmlChar*)"name", (xmlChar*)"ToggleMaximizeFull");
  }
  else if(a == TITLEBAR_SHADE) {
    n = xmlNewChild(n, NULL, (xmlChar*)"action", NULL);
    xmlSetProp(n, (xmlChar*)"name", (xmlChar*)"ToggleShade");
  }
  else {
    xmlNodePtr c = saved_custom->children;

    while(c) {
      xmlAddChild(n, xmlCopyNode(c, 1));
      c = c->next;
    }
  }

  tree_apply();
}


//------------------------- appearance -------------------------------

static RrFont* read_font(Fm::FontButton* button, const gchar* place, gboolean def);
static RrFont* write_font(Fm::FontButton* button, const gchar* place);

void MainDialog::appearance_setup_tab() {
  gchar* layout;
  RrFont* f;

  ui.window_border->setChecked(tree_get_bool("theme/keepBorder", TRUE));
  ui.animate_iconify->setChecked(tree_get_bool("theme/animateIconify", TRUE));

  layout = tree_get_string("theme/titleLayout", "NLIMC");
  ui.title_layout->setText(layout);
  // preview_update_set_title_layout(layout); // FIXME
  g_free(layout);

  f = read_font(ui.font_active, "ActiveWindow", TRUE);
  // FIXME preview_update_set_active_font(f);

  f = read_font(ui.font_inactive, "InactiveWindow", TRUE);
  // FIXME preview_update_set_inactive_font(f);

  f = read_font(ui.font_menu_header, "MenuHeader", TRUE);
  // FIXME preview_update_set_menu_header_font(f);

  f = read_font(ui.font_menu_item, "MenuItem", TRUE);
  // FIXME preview_update_set_menu_item_font(f);

  if(!(f = read_font(ui.font_active_display, "ActiveOnScreenDisplay", FALSE))) {
    f = read_font(ui.font_active_display, "OnScreenDisplay", TRUE);
    tree_delete_node("theme/font:place=OnScreenDisplay");
  }

  // FIXME preview_update_set_osd_active_font(f);

  f = read_font(ui.font_inactive_display, "InactiveOnScreenDisplay", TRUE);
  // FIXME preview_update_set_osd_inactive_font(f);
}

void MainDialog::on_window_border_toggled(bool checked) {
  tree_set_bool("theme/keepBorder", checked);
}

void MainDialog::on_animate_iconify_toggled(bool checked) {
  tree_set_bool("theme/animateIconify", checked);
}

void MainDialog::on_title_layout_textChanged(const QString & text) {
#if 0
// FIXME
  gchar* layout;
  gchar* it, *it2;
  gboolean n, d, s, l, i, m, c;

  layout = g_strdup(gtk_entry_get_text(w));

  n = d = s = l = i = m = c = FALSE;

  for(it = layout; *it; ++it) {
    gboolean* b;

    switch(*it) {
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

    if(!b || *b) {
      /* drop the letter */
      for(it2 = it; *it2; ++it2)
        *it2 = *(it2 + 1);
    }
    else {
      *it = toupper(*it);
      *b = TRUE;
    }
  }

  gtk_entry_set_text(w, layout);
  tree_set_string("theme/titleLayout", layout);
  preview_update_set_title_layout(layout);

  g_free(layout);
#endif
}

void MainDialog::on_font_active_changed() {
  // FIXME preview_update_set_active_font(write_font(w, "ActiveWindow"));
}

void MainDialog::on_font_inactive_changed() {
  // FIXME preview_update_set_inactive_font(write_font(w, "InactiveWindow"));
}

void MainDialog::on_font_menu_header_changed() {
  // FIXME preview_update_set_menu_header_font(write_font(w, "MenuHeader"));
}

void MainDialog::on_font_menu_item_changed() {
  // FIXME preview_update_set_menu_item_font(write_font(w, "MenuItem"));
}

void MainDialog::on_font_active_display_changed() {
  // FIXME preview_update_set_osd_active_font(write_font(w, "ActiveOnScreenDisplay"));
}

void MainDialog::on_font_inactive_display_changed() {
  // FIXME preview_update_set_osd_inactive_font
  // FIXME (write_font(w, "InactiveOnScreenDisplay"));
}

static RrFont* read_font(Fm::FontButton* button, const gchar* place,
                         gboolean use_default) {
  RrFont* font;
  gchar* fontstring, *node;
  gchar* name, **names;
  gchar* size;
  gchar* weight;
  gchar* slant;

  RrFontWeight rr_weight = RR_FONTWEIGHT_NORMAL;
  RrFontSlant rr_slant = RR_FONTSLANT_NORMAL;

  mapping = TRUE;

  node = g_strdup_printf("theme/font:place=%s/name", place);
  name = tree_get_string(node, use_default ? "Sans" : NULL);
  g_free(node);

  if(name[0] == '\0') {
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
  if(!g_ascii_strcasecmp(weight, "normal")) {
    g_free(weight); weight = g_strdup("");
  }

  if(!g_ascii_strcasecmp(slant, "normal")) {
    g_free(slant); slant = g_strdup("");
  }

  QFont qfont;
  qfont.setFamily(name);
  // FIXME qfont.setWeight(weight);
  qfont.setPointSize(atoi(size));

  button->setFont(qfont);

  if(!g_ascii_strcasecmp(weight, "Bold")) rr_weight = RR_FONTWEIGHT_BOLD;

  if(!g_ascii_strcasecmp(slant, "Italic")) rr_slant = RR_FONTSLANT_ITALIC;

  if(!g_ascii_strcasecmp(slant, "Oblique")) rr_slant = RR_FONTSLANT_OBLIQUE;

  font = RrFontOpen(rrinst, name, atoi(size), rr_weight, rr_slant);
  // g_free(fontstring);
  g_free(slant);
  g_free(weight);
  g_free(size);
  g_free(name);

  return font;
}

static RrFont* write_font(Fm::FontButton* button, const gchar* place) {
  gchar* c;
  gchar* font, *node;
  const gchar* size = NULL;
  const gchar* bold = NULL;
  const gchar* italic = NULL;

  RrFontWeight weight = RR_FONTWEIGHT_NORMAL;
  RrFontSlant slant = RR_FONTSLANT_NORMAL;

  QFont qfont = button->font();
#if 0
  // FIXME
  font = g_strdup(gtk_font_button_get_font_name(w));

  while((c = strrchr(font, ' '))) {
    if(!bold && !italic && !size && atoi(c + 1))
      size = c + 1;
    else if(!bold && !italic && !g_ascii_strcasecmp(c + 1, "italic"))
      italic = c + 1;
    else if(!bold && !g_ascii_strcasecmp(c + 1, "bold"))
      bold = c + 1;
    else
      break;

    *c = '\0';
  }

  if(!bold) bold = "Normal";

  if(!italic) italic = "Normal";

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

  if(!g_ascii_strcasecmp(bold, "Bold")) weight = RR_FONTWEIGHT_BOLD;

  if(!g_ascii_strcasecmp(italic, "Italic")) slant = RR_FONTSLANT_ITALIC;

  if(!g_ascii_strcasecmp(italic, "Oblique")) slant = RR_FONTSLANT_OBLIQUE;

  return RrFontOpen(rrinst, font, atoi(size), weight, slant);

  g_free(font);
#endif

}

//-------------------------- desktops ------------------------------

static int num_desktops;
static GList* desktop_names;

static void desktops_read_names();
static void desktops_write_names();
static void desktops_write_number();

static void enable_stuff();

void MainDialog::desktops_setup_tab() {
  num_desktops = tree_get_int("desktops/number", 4);
  ui.desktop_num->setValue(num_desktops);

  gint i;

#if 0
  // FIXME
  GtkWidget* w;
  GtkCellRenderer* render;
  GtkTreeViewColumn* column;

  w = get_widget("desktop_names");
  desktop_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_BOOLEAN);
  gtk_tree_view_set_model(GTK_TREE_VIEW(w), GTK_TREE_MODEL(desktop_store));
  g_object_unref(desktop_store);

  gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(w)),
                              GTK_SELECTION_SINGLE);

  render = gtk_cell_renderer_text_new();
  g_signal_connect(render, "edited",
                   G_CALLBACK(on_desktop_names_cell_edited),
                   NULL);

  column = gtk_tree_view_column_new_with_attributes
           ("Name", render, "text", 0, "editable", 1, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(w), column);

  desktops_read_names();
#endif

  i = tree_get_int("desktops/popupTime", 875);
  ui.desktop_popup->setChecked(i != 0);
  ui.desktop_popup_time->setValue(i ? i : 875);
  // FIXME enable_stuff();
}

/* FIXME
static void enable_stuff() {
  GtkWidget* w;
  gboolean b;

  w = get_widget("desktop_popup");
  b = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
  w = get_widget("desktop_popup_time");
  gtk_widget_set_sensitive(w, b);
}
*/

void MainDialog::on_desktop_num_valueChanged(int newValue) {
  num_desktops = newValue;
  desktops_write_number();
  desktops_read_names();
}

#if 0
// FIXME

static void MainDialog::on_desktop_names_cell_edited(GtkCellRendererText* cell,
    const gchar* path_string,
    const gchar* new_text,
    gpointer data) {
  GtkTreePath* path;
  GtkTreeIter it;
  gchar* old_text;
  GList* lit;
  gint i;

  path = gtk_tree_path_new_from_string(path_string);
  gtk_tree_model_get_iter(GTK_TREE_MODEL(desktop_store), &it, path);

  gtk_tree_model_get(GTK_TREE_MODEL(desktop_store), &it, 0, &old_text, -1);
  g_free(old_text);

  i = gtk_tree_path_get_indices(path)[0];
  lit = g_list_nth(desktop_names, i);

  g_free(lit->data);
  lit->data = g_strdup(new_text);

  if(new_text[0])  /* not empty */
    gtk_list_store_set(desktop_store, &it, 0, lit->data, -1);
  else
    gtk_list_store_set(desktop_store, &it, 0, _("(Unnamed desktop)"), -1);

  desktops_write_names();
}

#endif

static void desktops_read_names() {
#if 0
  // FIXME

  GtkTreeIter it;
  xmlNodePtr n;
  gint i;
  GList* lit;

  gtk_list_store_clear(desktop_store);

  for(lit = desktop_names; lit; lit = g_list_next(lit))
    g_free(lit->data);

  g_list_free(desktop_names);
  desktop_names = NULL;

  i = 0;
  n = tree_get_node("desktops/names", NULL)->children;

  while(n) {
    gchar* name;

    if(!xmlStrcmp(n->name, (const xmlChar*)"name")) {
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

  while(i < num_desktops) {
    gchar* name = g_strdup("");

    desktop_names = g_list_append(desktop_names, name);

    gtk_list_store_append(desktop_store, &it);
    gtk_list_store_set(desktop_store, &it,
                       0, _("(Unnamed desktop)"),
                       1, TRUE,
                       -1);
    ++i;
  }
#endif
}

static void desktops_write_names() {
#if 0
  //FIXME

  gchar** s;
  GList* lit;
  xmlNodePtr n, c;
  gint num = 0, last = -1;

  n = tree_get_node("desktops/names", NULL);

  while((c = n->children)) {
    xmlUnlinkNode(c);
    xmlFreeNode(c);
  }

  for(lit = desktop_names; lit; lit = g_list_next(lit)) {
    if(((gchar*)lit->data)[0])  /* not empty */
      last = num;

    ++num;
  }

  num = 0;

  for(lit = desktop_names; lit && num <= last; lit = g_list_next(lit)) {
    xmlNewTextChild(n, NULL, "name", lit->data);
    ++num;
  }

  tree_apply();

  /* make openbox re-set the property */
  XDeleteProperty(GDK_DISPLAY(), GDK_ROOT_WINDOW(),
                  gdk_x11_get_xatom_by_name("_NET_DESKTOP_NAMES"));
#endif
}

static void desktops_write_number() {
  XEvent ce;
  tree_set_int("desktops/number", num_desktops);
  ce.xclient.type = ClientMessage;
  ce.xclient.message_type =
    XInternAtom(QX11Info::display(), "_NET_NUMBER_OF_DESKTOPS", False);
  ce.xclient.display = QX11Info::display();
  ce.xclient.window = QX11Info::appRootWindow();
  ce.xclient.format = 32;
  ce.xclient.data.l[0] = num_desktops;
  ce.xclient.data.l[1] = 0;
  ce.xclient.data.l[2] = 0;
  ce.xclient.data.l[3] = 0;
  ce.xclient.data.l[4] = 0;
  XSendEvent(QX11Info::display(), QX11Info::appRootWindow(), FALSE,
             SubstructureNotifyMask | SubstructureRedirectMask,
             &ce);
}

void MainDialog::on_desktop_popup_toggled(bool checked) {
  if(checked) {
    tree_set_int("desktops/popupTime", ui.desktop_popup_time->value());
  }
  else
    tree_set_int("desktops/popupTime", 0);
  // FIXME enable_stuff();
}

void MainDialog::on_desktop_popup_time_valueChanged(int newValue) {
  tree_set_int("desktops/popupTime", newValue);
}

//------------------------- dock -------------------------------

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
  GtkWidget* w, *s;
  gboolean b;

  w = get_widget("dock_position");
  b = gtk_option_menu_get_history(GTK_OPTION_MENU(w)) == POSITION_FLOATING;

  s = get_widget("dock_float_x");
  gtk_widget_set_sensitive(s, b);
  s = get_widget("dock_float_y");
  gtk_widget_set_sensitive(s, b);
  s = get_widget("dock_float_label");
  gtk_widget_set_sensitive(s, b);
  s = get_widget("dock_float_label_x");
  gtk_widget_set_sensitive(s, b);
  s = get_widget("dock_nostrut");
  gtk_widget_set_sensitive(s, !b);

  w = get_widget("dock_hide");
  b = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));

  s = get_widget("dock_hide_delay");
  gtk_widget_set_sensitive(s, b);
  s = get_widget("dock_hide_label");
  gtk_widget_set_sensitive(s, b);
  s = get_widget("dock_hide_label_units");
  gtk_widget_set_sensitive(s, b);
  s = get_widget("dock_show_delay");
  gtk_widget_set_sensitive(s, b);
  s = get_widget("dock_show_label");
  gtk_widget_set_sensitive(s, b);
  s = get_widget("dock_show_label_units");
  gtk_widget_set_sensitive(s, b);
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

//-------------------------- theme ----------------------------------

static void add_theme_dir(const gchar* dirname);

void MainDialog::theme_setup_tab() {
#if 0
  GtkCellRenderer* render;
  GtkTreeViewColumn* column;
  GtkTreeSelection* select;
  GtkWidget* w;

  mapping = TRUE;

  w = get_widget("theme_names");

  /* widget setup */
  theme_store = gtk_list_store_new(3,
                                   /* the theme name */
                                   G_TYPE_STRING,
                                   /* the theme preview */
                                   GDK_TYPE_PIXBUF,
                                   /* alignment of the preview */
                                   G_TYPE_FLOAT);
  gtk_tree_view_set_model(GTK_TREE_VIEW(w), GTK_TREE_MODEL(theme_store));
  preview_update_set_tree_view(GTK_TREE_VIEW(w), theme_store);
  g_object_unref(theme_store);

  gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(w)),
                              GTK_SELECTION_SINGLE);

  /* text column for the names */
  render = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes
           ("Name", render, "text", 0, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(w), column);

  /* pixbuf column, for theme previews */
  render = gtk_cell_renderer_pixbuf_new();
  column = gtk_tree_view_column_new_with_attributes
           ("Preview", render, "pixbuf", 1, "xalign", 2, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(w), column);

  /* setup the selection handler */
  select = gtk_tree_view_get_selection(GTK_TREE_VIEW(w));
  gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
  g_signal_connect(G_OBJECT(select), "changed",
                   G_CALLBACK(on_theme_names_selection_changed),
                   NULL);

  mapping = FALSE;
#endif
}

#if 0
static void MainDialog::on_theme_names_selection_changed(GtkTreeSelection* sel,
    gpointer data) {
  GtkTreeIter iter;
  GtkTreeModel* model;
  const gchar* name;



  if(gtk_tree_selection_get_selected(sel, &model, &iter)) {
    gtk_tree_model_get(model, &iter, 0, &name, -1);
  }

  if(name)
    tree_set_string("theme/name", name);
}

void MainDialog::on_install_theme_clicked(GtkButton* w, gpointer data) {
  GtkWidget* d;
  gint r;
  gchar* path = NULL;
  GtkFileFilter* filter;

  d = gtk_file_chooser_dialog_new(_("Choose an Openbox theme"),
                                  GTK_WINDOW(mainwin),
                                  GTK_FILE_CHOOSER_ACTION_OPEN,
                                  GTK_STOCK_OK, GTK_RESPONSE_OK,
                                  GTK_STOCK_CANCEL, GTK_RESPONSE_NONE,
                                  NULL);

  gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(d), FALSE);
  filter = gtk_file_filter_new();
  gtk_file_filter_set_name(filter, _("Openbox theme archives"));
  gtk_file_filter_add_pattern(filter, "*.obt");
  //gtk_file_filter_add_pattern(filter, "*.tgz");
  //gtk_file_filter_add_pattern(filter, "*.tar.gz");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(d), filter);

  r = gtk_dialog_run(GTK_DIALOG(d));

  if(r == GTK_RESPONSE_OK)
    path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d));

  gtk_widget_destroy(d);

  if(path != NULL) {
    theme_install(path);
    g_free(path);
  }
}

void MainDialog::on_theme_archive_clicked(GtkButton* w, gpointer data) {
  GtkWidget* d;
  gint r;
  gchar* path = NULL;

  d = gtk_file_chooser_dialog_new(_("Choose an Openbox theme"),
                                  GTK_WINDOW(mainwin),
                                  GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                  GTK_STOCK_OK, GTK_RESPONSE_OK,
                                  GTK_STOCK_CANCEL, GTK_RESPONSE_NONE,
                                  NULL);

  gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(d), TRUE);
  r = gtk_dialog_run(GTK_DIALOG(d));

  if(r == GTK_RESPONSE_OK)
    path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d));

  gtk_widget_destroy(d);

  if(path != NULL) {
    archive_create(path);
    g_free(path);
  }
}
#endif

