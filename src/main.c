/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   main.c for ObConf, the configuration tool for Openbox
   Copyright (c) 2003-2008   Dana Jansens
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
#include "archive.h"
#include "theme.h"
#include "appearance.h"
#include "windows.h"
#include "margins.h"
#include "mouse.h"
#include "desktops.h"
#include "dock.h"
#include "preview_update.h"
#include "gettext.h"

#include <gdk/gdkx.h>
#define SN_API_NOT_YET_FROZEN
#include <libsn/sn.h>
#undef SN_API_NOT_YET_FROZEN
#include <stdlib.h>

GtkWidget *mainwin = NULL;

GladeXML *glade;
xmlDocPtr doc;
xmlNodePtr root;
RrInstance *rrinst;
gchar *obc_config_file = NULL;
ObtPaths *paths;
ObtXmlInst *parse_i;

static gchar *obc_theme_install = NULL;
static gchar *obc_theme_archive = NULL;

void obconf_error(gchar *msg, gboolean modal)
{
    GtkWidget *d;

    d = gtk_message_dialog_new(mainwin ? GTK_WINDOW(mainwin) : NULL,
                               GTK_DIALOG_DESTROY_WITH_PARENT,
                               GTK_MESSAGE_ERROR,
                               GTK_BUTTONS_CLOSE,
                               "%s", msg);
    gtk_window_set_title(GTK_WINDOW(d), "ObConf Error");
    if (modal)
        gtk_dialog_run(GTK_DIALOG(d));
    else {
        g_signal_connect_swapped(GTK_OBJECT(d), "response",
                                 G_CALLBACK(gtk_widget_destroy),
                                 GTK_OBJECT(d));
        gtk_widget_show(d);
    }
}

static void print_version()
{
    g_print("ObConf %s\n", PACKAGE_VERSION);
    g_print(_("Copyright (c)"));
    g_print(" 2003-2008   Dana Jansens\n");
    g_print(_("Copyright (c)"));
    g_print(" 2003        Tim Riley\n");
    g_print(_("Copyright (c)"));
    g_print(" 2007        Javeed Shaikh\n\n");
    g_print("This program comes with ABSOLUTELY NO WARRANTY.\n");
    g_print("This is free software, and you are welcome to redistribute it\n");
    g_print("under certain conditions. See the file COPYING for details.\n\n");

    exit(EXIT_SUCCESS);
}

static void print_help()
{
    g_print(_("Syntax: obconf [options] [ARCHIVE.obt]\n"));
    g_print(_("\nOptions:\n"));
    g_print(_("  --help                Display this help and exit\n"));
    g_print(_("  --version             Display the version and exit\n"));
    g_print(_("  --install ARCHIVE.obt Install the given theme archive and select it\n"));
    g_print(_("  --archive THEME       Create a theme archive from the given theme directory\n"));
    g_print(_("  --config-file FILE    Specify the path to the config file to use\n"));
    g_print(_("\nPlease report bugs at %s\n\n"), PACKAGE_BUGREPORT);
    
    exit(EXIT_SUCCESS);
}

static void parse_args(int argc, char **argv)
{
    int i;

    for (i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--help"))
            print_help();
        if (!strcmp(argv[i], "--version"))
            print_version();
        else if (!strcmp(argv[i], "--install")) {
            if (i == argc - 1) /* no args left */
                g_printerr(_("--install requires an argument\n"));
            else
                obc_theme_install = argv[++i];
        }
        else if (!strcmp(argv[i], "--archive")) {
            if (i == argc - 1) /* no args left */
                g_printerr(_("--archive requires an argument\n"));
            else
                obc_theme_archive = argv[++i];
        }
        else if (!strcmp(argv[i], "--config-file")) {
            if (i == argc - 1) /* no args left */
                g_printerr(_("--config-file requires an argument\n"));
            else
                obc_config_file = argv[++i];
        } else
            obc_theme_install = argv[i];
    }
}

static gboolean get_all(Window win, Atom prop, Atom type, gint size,
                        guchar **data, guint *num)
{
    gboolean ret = FALSE;
    gint res;
    guchar *xdata = NULL;
    Atom ret_type;
    gint ret_size;
    gulong ret_items, bytes_left;

    res = XGetWindowProperty(GDK_DISPLAY(), win, prop, 0l, G_MAXLONG,
                             FALSE, type, &ret_type, &ret_size,
                             &ret_items, &bytes_left, &xdata);
    if (res == Success) {
        if (ret_size == size && ret_items > 0) {
            guint i;

            *data = g_malloc(ret_items * (size / 8));
            for (i = 0; i < ret_items; ++i)
                switch (size) {
                case 8:
                    (*data)[i] = xdata[i];
                    break;
                case 16:
                    ((guint16*)*data)[i] = ((gushort*)xdata)[i];
                    break;
                case 32:
                    ((guint32*)*data)[i] = ((gulong*)xdata)[i];
                    break;
                default:
                    g_assert_not_reached(); /* unhandled size */
                }
            *num = ret_items;
            ret = TRUE;
        }
        XFree(xdata);
    }
    return ret;
}

static gboolean prop_get_string_utf8(Window win, Atom prop, gchar **ret)
{
    gchar *raw;
    gchar *str;
    guint num;

    if (get_all(win, prop,
                gdk_x11_get_xatom_by_name("UTF8_STRING"),
                8,(guchar**)&raw, &num))
    {
        str = g_strndup(raw, num); /* grab the first string from the list */
        g_free(raw);
        if (g_utf8_validate(str, -1, NULL)) {
            *ret = str;
            return TRUE;
        }
        g_free(str);
    }
    return FALSE;
}

int main(int argc, char **argv)
{
    gchar *p;
    gboolean exit_with_error = FALSE;

    bindtextdomain(PACKAGE_NAME, LOCALEDIR);
    bind_textdomain_codeset(PACKAGE_NAME, "UTF-8");
    textdomain(PACKAGE_NAME);

    gtk_init(&argc, &argv);
    parse_args(argc, argv);

    if (obc_theme_archive) {
        archive_create(obc_theme_archive);
        return;
    }

    p = g_build_filename(GLADEDIR, "obconf.glade", NULL);
    glade = glade_xml_new(p, NULL, NULL);
    g_free(p);

    if (!glade) {
        obconf_error(_("Failed to load the obconf.glade interface file. You have probably failed to install ObConf properly."), TRUE);
        exit_with_error = TRUE;
    }

    paths = obt_paths_new();
    parse_i = obt_xml_instance_new();
    rrinst = RrInstanceNew(GDK_DISPLAY(), gdk_x11_get_default_screen());

    if (!obc_config_file) {
        gchar *p;

        if (prop_get_string_utf8(GDK_ROOT_WINDOW(),
                                 gdk_x11_get_xatom_by_name("_OB_CONFIG_FILE"),
                                 &p))
        {
            obc_config_file = g_filename_from_utf8(p, -1, NULL, NULL, NULL);
            g_free(p);
        }
    }

    xmlIndentTreeOutput = 1;
    if (!((obc_config_file &&
           obt_xml_load_file(parse_i, obc_config_file, "openbox_config")) ||
          obt_xml_load_config_file(parse_i, "openbox", "rc.xml",
                                   "openbox_config")))
    {
        obconf_error(_("Failed to load an rc.xml. You have probably failed to install Openbox properly."), TRUE);
        exit_with_error = TRUE;
    }
    else {
        doc = obt_xml_doc(parse_i);
        root = obt_xml_root(parse_i);
    }

    /* look for parsing errors */
    {
        xmlErrorPtr e = xmlGetLastError();
        if (e) {
            char *a = g_strdup_printf
                (_("Error while parsing the Openbox configuration file.  Your configuration file is not valid XML.\n\nMessage: %s"),
                 e->message);
            obconf_error(a, TRUE);
            g_free(a);
            exit_with_error = TRUE;
        }
    }

    if (!exit_with_error) {
        glade_xml_signal_autoconnect(glade);

        {
            gchar *s = g_strdup_printf
                ("<span weight=\"bold\" size=\"xx-large\">ObConf %s</span>",
                 PACKAGE_VERSION);
            gtk_label_set_markup(GTK_LABEL
                                 (glade_xml_get_widget(glade, "title_label")),
                                 s);
            g_free(s);
        }

        theme_setup_tab();
        appearance_setup_tab();
        windows_setup_tab();
        moveresize_setup_tab();
        mouse_setup_tab();
        desktops_setup_tab();
        margins_setup_tab();
        dock_setup_tab();

        mainwin = get_widget("main_window");

        if (obc_theme_install)
            theme_install(obc_theme_install);
        else
            theme_load_all();

        /* the main window is not shown here ! it is shown when the theme
           previews are completed */
        gtk_main();

        preview_update_set_active_font(NULL);
        preview_update_set_inactive_font(NULL);
        preview_update_set_menu_header_font(NULL);
        preview_update_set_menu_item_font(NULL);
        preview_update_set_osd_active_font(NULL);
        preview_update_set_osd_inactive_font(NULL);
        preview_update_set_title_layout(NULL);
    }

    RrInstanceFree(rrinst);
    obt_xml_instance_unref(parse_i);
    obt_paths_unref(paths);

    xmlFreeDoc(doc);
    return 0;
}

gboolean on_main_window_delete_event(GtkWidget *w, GdkEvent *e, gpointer d)
{
    gtk_main_quit();
    return FALSE;
}

void on_close_clicked()
{
    gtk_main_quit();
}

void obconf_show_main()
{
    SnDisplay *sn_d;
    SnLauncheeContext *sn_cx;

    if (GTK_WIDGET_VISIBLE(mainwin)) return;

    gtk_widget_show_all(mainwin);

    sn_d = sn_display_new(GDK_DISPLAY_XDISPLAY(gdk_display_get_default()),
                          NULL, NULL);

    sn_cx = sn_launchee_context_new_from_environment
        (sn_d, gdk_screen_get_number(gdk_display_get_default_screen
                                     (gdk_display_get_default())));

    if (sn_cx)
        sn_launchee_context_setup_window
            (sn_cx, GDK_WINDOW_XWINDOW(GDK_WINDOW(mainwin->window)));

    if (sn_cx)
        sn_launchee_context_complete(sn_cx);

    if (sn_cx)
        sn_launchee_context_unref(sn_cx);
    sn_display_unref(sn_d);
}
