/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   theme.h for ObConf, the configuration tool for Openbox
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
#include "preview_update.h"
#include "gettext.h"
#include "archive.h"
#include "theme.h"
#include "preview.h"

static gboolean mapping = FALSE;

static GList* themes = NULL;

static void add_theme_dir(const gchar *dirname);

void theme_install(const gchar *path)
{
    gchar *name;

    if ((name = archive_install(path)))
        tree_set_string("theme/name", name);
    g_free(name);

    theme_load_all();
}

void theme_load_all()
{
#if 0
    gchar *name;
    gchar *p;
    GList *it, *next;
    gint i;
    GtkWidget *w;
    RrFont *active, *inactive, *menu_t, *menu_i, *osd;

    mapping = TRUE;

    w = get_widget("theme_names");
    name = tree_get_string("theme/name", "TheBear");

    for (it = themes; it; it = g_list_next(it))
        g_free(it->data);
    g_list_free(themes);
    themes = NULL;

    p = g_build_filename(g_get_home_dir(), ".themes", NULL);
    add_theme_dir(p);
    g_free(p);

    {
        GSList *it;
        for (it = obt_paths_data_dirs(paths); it; it = g_slist_next(it)) {
            p = g_build_filename(it->data, "themes", NULL);
            add_theme_dir(p);
            g_free(p);
        }
    }

    add_theme_dir(THEMEDIR);

    themes = g_list_sort(themes, (GCompareFunc) strcasecmp);

    gtk_list_store_clear(theme_store);

    /* return to regular scheduled programming */
    i = 0;
    for (it = themes; it; it = next) {
        GtkTreeIter iter;

        next = g_list_next(it);

        /* remove duplicates */
        if (next && !strcmp(it->data, next->data)) {
            g_free(it->data);
            themes = g_list_delete_link(themes, it);
            continue;
        }

        gtk_list_store_append(theme_store, &iter);
        gtk_list_store_set(theme_store, &iter,
                           0, it->data, /* the theme's name */
                           1, NULL,     /* the preview is set later */
                           2, 1.0,      /* all previews are right-aligned */
                           -1);

        if(!strcmp(name, it->data)) {
            GtkTreePath *path;

            path = gtk_tree_path_new_from_indices(i, -1);
            gtk_tree_view_set_cursor(GTK_TREE_VIEW(w), path, NULL, FALSE);
            gtk_tree_path_free(path);
        }

        ++i;
    }

    preview_update_all();

    g_free(name);

    mapping = FALSE;
#endif
}

static void add_theme_dir(const gchar *dirname)
{
    GDir *dir;
    const gchar *n;

    if ((dir = g_dir_open(dirname, 0, NULL))) {
        while ((n = g_dir_read_name(dir))) {
            {
                gchar *full;
                full = g_build_filename(dirname, n, "openbox-3",
                                        "themerc", NULL);
                if (!g_file_test(full, GFileTest(G_FILE_TEST_IS_REGULAR |G_FILE_TEST_IS_SYMLINK)))
                    n = NULL;
                g_free(full);
            }

            if (n) {
                themes = g_list_append(themes, g_strdup(n));
            }
        }
        g_dir_close(dir);
    }
}

