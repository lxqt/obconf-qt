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

static int num_desktops;

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

#endif

  desktops_read_names();

  i = tree_get_int("desktops/popupTime", 875);
  ui.desktop_popup->setChecked(i != 0);
  ui.desktop_popup_time->setValue(i ? i : 875);
  // FIXME enable_stuff();
}

/* FIXME
 * static void enable_stuff() {
 *  GtkWidget* w;
 *  gboolean b;
 *
 *  w = get_widget("desktop_popup");
 *  b = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
 *  w = get_widget("desktop_popup_time");
 *  gtk_widget_set_sensitive(w, b);
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

void MainDialog::desktops_read_names() {
  xmlNodePtr n;
  gint i;

  ui.desktop_names->clear();

  i = 0;
  n = tree_get_node("desktops/names", NULL)->children;

  while(n) {
    gchar* name;

    if(!xmlStrcmp(n->name, (const xmlChar*)"name")) {
      name = obt_xml_node_string(n);
      QString desktop_name = QString::fromUtf8(name);

      if(desktop_name.isEmpty())
        desktop_name = tr("(Unnamed desktop)");

      ui.desktop_names->addItem(desktop_name);
      ++i;
    }

    n = n->next;
  }

  while(i < num_desktops) {
    ui.desktop_names->addItem(tr("(Unnamed desktop)"));
    ++i;
  }
}

void MainDialog::desktops_write_names() {
  gchar** s;
  xmlNodePtr n, c;
  gint num = 0, last = -1;

  // delete all existing keys
  n = tree_get_node("desktops/names", NULL);

  while((c = n->children)) {
    xmlUnlinkNode(c);
    xmlFreeNode(c);
  }

  int i;

  for(i = 0; i < ui.desktop_names->count(); ++i) {
    QListWidgetItem* item = ui.desktop_names->item(i);
    QString text = item->text();
    xmlNewTextChild(n, NULL, (xmlChar*)"name", (xmlChar*)text.toUtf8().constData());
  }

  tree_apply();
  /* make openbox re-set the property */
  XDeleteProperty(QX11Info::display(), QX11Info::appRootWindow(),
                  XInternAtom(QX11Info::display(), "_NET_DESKTOP_NAMES", False));
}

void MainDialog::desktops_write_number() {
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
