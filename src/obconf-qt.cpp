#include "obconf-qt.h"
#include <glib.h>
#include <glib/gi18n.h>

#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLocale>
#include <QX11Info>
#include <QMessageBox>
#include "maindialog.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "main.h"
#include "archive.h"
#include "preview_update.h"
#include <stdlib.h>

using namespace Obconf;

xmlDocPtr doc;
xmlNodePtr root;
RrInstance* rrinst;
gchar* obc_config_file = NULL;
ObtPaths* paths;
ObtXmlInst* parse_i;

static gchar* obc_theme_install = NULL;
static gchar* obc_theme_archive = NULL;

void obconf_error(gchar* msg, gboolean modal) {
  // FIXME: we did not handle modal
  QMessageBox::critical(NULL, QObject::tr("ObConf Error"), QString::fromUtf8(msg));
}

static void print_version() {
  // g_print("ObConf %s\n", PACKAGE_VERSION);
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

static void print_help() {
  g_print(_("Syntax: obconf [options] [ARCHIVE.obt]\n"));
  g_print(_("\nOptions:\n"));
  g_print(_("  --help                Display this help and exit\n"));
  g_print(_("  --version             Display the version and exit\n"));
  g_print(_("  --install ARCHIVE.obt Install the given theme archive and select it\n"));
  g_print(_("  --archive THEME       Create a theme archive from the given theme directory\n"));
  g_print(_("  --config-file FILE    Specify the path to the config file to use\n"));
  // g_print(_("\nPlease report bugs at %s\n\n"), PACKAGE_BUGREPORT);

  exit(EXIT_SUCCESS);
}

static void parse_args(int argc, char** argv) {
  int i;

  for(i = 1; i < argc; ++i) {
    if(!strcmp(argv[i], "--help"))
      print_help();

    if(!strcmp(argv[i], "--version"))
      print_version();
    else if(!strcmp(argv[i], "--install")) {
      if(i == argc - 1)  /* no args left */
        g_printerr(_("--install requires an argument\n"));
      else
        obc_theme_install = argv[++i];
    }
    else if(!strcmp(argv[i], "--archive")) {
      if(i == argc - 1)  /* no args left */
        g_printerr(_("--archive requires an argument\n"));
      else
        obc_theme_archive = argv[++i];
    }
    else if(!strcmp(argv[i], "--config-file")) {
      if(i == argc - 1)  /* no args left */
        g_printerr(_("--config-file requires an argument\n"));
      else
        obc_config_file = argv[++i];
    }
    else
      obc_theme_install = argv[i];
  }
}

static gboolean get_all(Window win, Atom prop, Atom type, gint size,
                        guchar** data, guint* num) {
  gboolean ret = FALSE;
  gint res;
  guchar* xdata = NULL;
  Atom ret_type;
  gint ret_size;
  gulong ret_items, bytes_left;

  res = XGetWindowProperty(QX11Info::display(), win, prop, 0l, G_MAXLONG,
                           FALSE, type, &ret_type, &ret_size,
                           &ret_items, &bytes_left, &xdata);

  if(res == Success) {
    if(ret_size == size && ret_items > 0) {
      guint i;
      *data = (guchar*)g_malloc(ret_items * (size / 8));

      for(i = 0; i < ret_items; ++i)
        switch(size) {
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

static gboolean prop_get_string_utf8(Window win, Atom prop, gchar** ret) {
  gchar* raw;
  gchar* str;
  guint num;

  if(get_all(win, prop, XInternAtom(QX11Info::display(), "UTF8_STRING", 0), 8, (guchar**)&raw, &num)) {
    str = g_strndup(raw, num); /* grab the first string from the list */
    g_free(raw);

    if(g_utf8_validate(str, -1, NULL)) {
      *ret = str;
      return TRUE;
    }

    g_free(str);
  }

  return FALSE;
}

#if 0
int obconf_main(int argc, char** argv) {
  gchar* p;
  gboolean exit_with_error = FALSE;

  parse_args(argc, argv);

  if(obc_theme_archive) {
    archive_create(obc_theme_archive);
    return;
  }

  p = g_build_filename(GLADEDIR, "obconf.glade", NULL);
  glade = glade_xml_new(p, NULL, NULL);
  g_free(p);

  if(!glade) {
    obconf_error(_("Failed to load the obconf.glade interface file. You have probably failed to install ObConf properly."), TRUE);
    exit_with_error = TRUE;
  }

  paths = obt_paths_new();
  parse_i = obt_xml_instance_new();
  rrinst = RrInstanceNew(GDK_DISPLAY(), gdk_x11_get_default_screen());

  if(!obc_config_file) {
    gchar* p;

    if(prop_get_string_utf8(GDK_ROOT_WINDOW(),
                            gdk_x11_get_xatom_by_name("_OB_CONFIG_FILE"),
                            &p)) {
      obc_config_file = g_filename_from_utf8(p, -1, NULL, NULL, NULL);
      g_free(p);
    }
  }

  xmlIndentTreeOutput = 1;

  if(!((obc_config_file &&
        obt_xml_load_file(parse_i, obc_config_file, "openbox_config")) ||
       obt_xml_load_config_file(parse_i, "openbox", "rc.xml",
                                "openbox_config"))) {
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

    if(e) {
      char* a = g_strdup_printf
                (_("Error while parsing the Openbox configuration file.  Your configuration file is not valid XML.\n\nMessage: %s"),
                 e->message);
      obconf_error(a, TRUE);
      g_free(a);
      exit_with_error = TRUE;
    }
  }

  if(!exit_with_error) {
    glade_xml_signal_autoconnect(glade);

    {
      gchar* s = g_strdup_printf
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

    if(obc_theme_install)
      theme_install(obc_theme_install);
    else
      theme_load_all();

    /* the main window is not shown here ! it is shown when the theme
      *           previews are completed */
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
#endif

int main(int argc, char** argv) {
  QApplication app(argc, argv);

  // load translations
  QTranslator qtTranslator, translator;
  // install the translations built-into Qt itself
  qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  app.installTranslator(&qtTranslator);
  // install our own tranlations
  translator.load("obconf-qt_" + QLocale::system().name(), PACKAGE_DATA_DIR "/translations");
  app.installTranslator(&translator);

  // load configurations
  gchar* p;
  gboolean exit_with_error = FALSE;

  parse_args(argc, argv);

  if(obc_theme_archive) {
    archive_create(obc_theme_archive);
    return 0;
  }

  paths = obt_paths_new();
  parse_i = obt_xml_instance_new();
  int screen = QX11Info::appScreen();
  rrinst = RrInstanceNew(QX11Info::display(), screen);
  if(!obc_config_file) {
    gchar* p;
    if(prop_get_string_utf8(QX11Info::appRootWindow(screen),
                            XInternAtom(QX11Info::display(), "_OB_CONFIG_FILE", 0), &p)) {
      obc_config_file = g_filename_from_utf8(p, -1, NULL, NULL, NULL);
      g_free(p);
    }
  }
  xmlIndentTreeOutput = 1;

  if(!((obc_config_file &&
        obt_xml_load_file(parse_i, obc_config_file, "openbox_config")) ||
       obt_xml_load_config_file(parse_i, "openbox", "rc.xml",
                                "openbox_config"))) {
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

    if(e) {
      char* a = g_strdup_printf
                (_("Error while parsing the Openbox configuration file.  Your configuration file is not valid XML.\n\nMessage: %s"),
                 e->message);
      obconf_error(a, TRUE);
      g_free(a);
      exit_with_error = TRUE;
    }
  }

  // build our GUI
  MainDialog dlg;
  if(obc_theme_install)
    dlg.theme_install(obc_theme_install);
  else
    dlg.theme_load_all();
  dlg.exec();

  /*
  preview_update_set_active_font(NULL);
  preview_update_set_inactive_font(NULL);
  preview_update_set_menu_header_font(NULL);
  preview_update_set_menu_item_font(NULL);
  preview_update_set_osd_active_font(NULL);
  preview_update_set_osd_inactive_font(NULL);
  preview_update_set_title_layout(NULL);
  */

  RrInstanceFree(rrinst);
  obt_xml_instance_unref(parse_i);
  obt_paths_unref(paths);
  xmlFreeDoc(doc);

  return 0;
}
