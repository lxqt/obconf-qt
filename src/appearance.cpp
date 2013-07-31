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

using namespace Obconf;

extern RrInstance* rrinst; // defined in obconf-qt.cpp

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

void MainDialog::on_title_layout_textChanged(const QString& text) {
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
