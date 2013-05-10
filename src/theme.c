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

static GList *themes;
static GtkListStore *theme_store;

static void add_theme_dir(const gchar *dirname);
static void on_theme_names_selection_changed(GtkTreeSelection *sel,
                                             gpointer data);

void theme_setup_tab()
{
    GtkCellRenderer *render;
    GtkTreeViewColumn *column;
    GtkTreeSelection *select;
    GtkWidget *w;

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
    g_object_unref (theme_store);

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
    select = gtk_tree_view_get_selection(GTK_TREE_VIEW (w));
    gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
    g_signal_connect (G_OBJECT(select), "changed",
                      G_CALLBACK(on_theme_names_selection_changed),
                      NULL);

    mapping = FALSE;
}

static void on_theme_names_selection_changed(GtkTreeSelection *sel,
                                             gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    const gchar *name;

    if (mapping) return;

    if(gtk_tree_selection_get_selected(sel, &model, &iter)) {
        gtk_tree_model_get(model, &iter, 0, &name, -1);
    }

    if(name)
      tree_set_string("theme/name", name);
}

void on_install_theme_clicked(GtkButton *w, gpointer data)
{
    GtkWidget *d;
    gint r;
    gchar *path = NULL;
    GtkFileFilter *filter;

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
    if (r == GTK_RESPONSE_OK)
        path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d));
    gtk_widget_destroy(d);

    if (path != NULL) {
        theme_install(path);
        g_free(path);
    }
}

void on_theme_archive_clicked(GtkButton *w, gpointer data)
{
    GtkWidget *d;
    gint r;
    gchar *path = NULL;

    d = gtk_file_chooser_dialog_new(_("Choose an Openbox theme"),
                                    GTK_WINDOW(mainwin),
                                    GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                    GTK_STOCK_OK, GTK_RESPONSE_OK,
                                    GTK_STOCK_CANCEL, GTK_RESPONSE_NONE,
                                    NULL);

    gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(d), TRUE);
    r = gtk_dialog_run(GTK_DIALOG(d));
    if (r == GTK_RESPONSE_OK)
        path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d));
    gtk_widget_destroy(d);

    if (path != NULL) {
        archive_create(path);
        g_free(path);
    }
}

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
                if (!g_file_test(full,
                                 G_FILE_TEST_IS_REGULAR |
                                 G_FILE_TEST_IS_SYMLINK))
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

