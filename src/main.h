/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   main.h for ObConf, the configuration tool for Openbox
   Copyright (c) 2003        Dana Jansens

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

#ifndef obconf__main_h
#define obconf__main_h

#include <obrender/render.h>
#include <obrender/instance.h>
#include <obt/xml.h>
#include <obt/paths.h>

#include <gtk/gtk.h>
#include <glade/glade-xml.h>

extern GladeXML *glade;
extern RrInstance *rrinst;
extern GtkWidget *mainwin;
extern gchar *obc_config_file;
extern ObtPaths *paths;
extern ObtXmlInst *parse_i;

#define get_widget(s) glade_xml_get_widget(glade, s)

void obconf_error(gchar *msg, gboolean model);
void obconf_show_main();

#endif
