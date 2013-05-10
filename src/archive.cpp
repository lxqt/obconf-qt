#include "theme.h"

#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define gtk_msg(type, args...) \
{                                                                        \
    GtkWidget *msgw;                                                     \
    msgw = gtk_message_dialog_new(GTK_WINDOW(mainwin),                   \
                                  GTK_DIALOG_DESTROY_WITH_PARENT |       \
                                  GTK_DIALOG_MODAL,                      \
                                  type,                                  \
                                  GTK_BUTTONS_OK,                        \
                                  args);                                 \
    gtk_dialog_run(GTK_DIALOG(msgw));                                    \
    gtk_widget_destroy(msgw);                                            \
}

static gchar *get_theme_dir();
static gboolean change_dir(const gchar *dir);
static gchar* name_from_dir(const gchar *dir);
static gchar* install_theme_to(const gchar *file, const gchar *to);
static gboolean create_theme_archive(const gchar *dir, const gchar *name,
                                     const gchar *to);

gchar* archive_install(const gchar *path)
{
    gchar *dest;
    gchar *name;

    if (!(dest = get_theme_dir()))
        return NULL;

    if ((name = install_theme_to(path, dest))) {
        gtk_msg(GTK_MESSAGE_INFO, _("\"%s\" was installed to %s"), name, dest);
    }

    g_free(dest);

    return name;
}

void archive_create(const gchar *path)
{
    gchar *name;
    gchar *dest;

    if (!(name = name_from_dir(path)))
        return;

    {
        gchar *file;
        file = g_strdup_printf("%s.obt", name);
        dest = g_build_path(G_DIR_SEPARATOR_S,
                            g_get_current_dir(), file, NULL);
        g_free(file);
    }

    if (create_theme_archive(path, name, dest))
        gtk_msg(GTK_MESSAGE_INFO, _("\"%s\" was successfully created"),
                dest);

    g_free(dest);
    g_free(name);
}

static gboolean create_theme_archive(const gchar *dir, const gchar *name,
                                     const gchar *to)
{
    gchar *glob;
    gchar **argv;
    gchar *errtxt = NULL;
    gchar *parentdir;
    gint exitcode;
    GError *e = NULL;

    glob = g_strdup_printf("%s/openbox-3/", name);

    parentdir = g_build_path(G_DIR_SEPARATOR_S, dir, "..", NULL);

    argv = g_new(gchar*, 9);
    argv[0] = g_strdup("tar");
    argv[1] = g_strdup("-c");
    argv[2] = g_strdup("-z");
    argv[3] = g_strdup("-f");
    argv[4] = g_strdup(to);
    argv[5] = g_strdup("-C");
    argv[6] = g_strdup(parentdir);
    argv[7] = g_strdup(glob);
    argv[8] = NULL;
    if (g_spawn_sync(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL,
                     NULL, &errtxt, &exitcode, &e))
    {
        if (exitcode != EXIT_SUCCESS)
            gtk_msg(GTK_MESSAGE_ERROR,
                    _("Unable to create the theme archive \"%s\".\nThe following errors were reported:\n%s"),
                    to, errtxt);

    }
    else
        gtk_msg(GTK_MESSAGE_ERROR, _("Unable to run the \"tar\" command: %s"),
                e->message);

    g_strfreev(argv);
    if (e) g_error_free(e);
    g_free(errtxt);
    g_free(parentdir);
    g_free(glob);
    return exitcode == EXIT_SUCCESS;
}

static gchar *get_theme_dir()
{
    gchar *dir;
    gint r;

    dir = g_build_path(G_DIR_SEPARATOR_S, g_get_home_dir(), ".themes", NULL);
    r = mkdir(dir, 0777);
    if (r == -1 && errno != EEXIST) {
        gtk_msg(GTK_MESSAGE_ERROR,
                _("Unable to create directory \"%s\": %s"),
                dir, strerror(errno));
        g_free(dir);
        dir = NULL;
    }

    return dir;
}

static gchar* name_from_dir(const gchar *dir)
{
    gchar *rc;
    struct stat st;
    gboolean r;

    rc = g_build_path(G_DIR_SEPARATOR_S, dir, "openbox-3", "themerc", NULL);

    r = (stat(rc, &st) == 0 && S_ISREG(st.st_mode));
    g_free(rc);

    if (!r) {
        gtk_msg(GTK_MESSAGE_ERROR,
                _("\"%s\" does not appear to be a valid Openbox theme directory"),
                dir);
        return NULL;
    }
    return g_path_get_basename(dir);
}

static gboolean change_dir(const gchar *dir)
{
    if (chdir(dir) == -1) {
        gtk_msg(GTK_MESSAGE_ERROR, _("Unable to move to directory \"%s\": %s"),
                dir, strerror(errno));
        return FALSE;
    }
    return TRUE;
}

static gchar* install_theme_to(const gchar *file, const gchar *to)
{
    gchar **argv;
    gchar *errtxt = NULL, *outtxt = NULL;
    gint exitcode;
    GError *e = NULL;
    gchar *name = NULL;

    argv = g_new(gchar*, 11);
    argv[0] = g_strdup("tar");
    argv[1] = g_strdup("-x");
    argv[2] = g_strdup("-v");
    argv[3] = g_strdup("-z");
    argv[4] = g_strdup("--wildcards");
    argv[5] = g_strdup("-f");
    argv[6] = g_strdup(file);
    argv[7] = g_strdup("-C");
    argv[8] = g_strdup(to);
    argv[9] = g_strdup("*/openbox-3/");
    argv[10] = NULL;
    if (!g_spawn_sync(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL,
                      &outtxt, &errtxt, &exitcode, &e))
        gtk_msg(GTK_MESSAGE_ERROR, _("Unable to run the \"tar\" command: %s"),
                e->message);
    g_strfreev(argv);
    if (e) g_error_free(e);

    if (exitcode != EXIT_SUCCESS)
        gtk_msg(GTK_MESSAGE_ERROR,
                _("Unable to extract the file \"%s\".\nPlease ensure that \"%s\" is writable and that the file is a valid Openbox theme archive.\nThe following errors were reported:\n%s"),
                file, to, errtxt);

    if (exitcode == EXIT_SUCCESS) {
        gchar **lines = g_strsplit(outtxt, "\n", 0);
        gchar **it;
        for (it = lines; *it; ++it) {
            gchar *l = *it;
            gboolean found = FALSE;

            while (*l && !found) {
                if (!strcmp(l, "/openbox-3/")) {
                    *l = '\0'; /* cut the string */
                    found = TRUE;
                }
                ++l;
            }

            if (found) {
                name = g_strdup(*it);
                break;
            }
        }
        g_strfreev(lines);
    }

    g_free(outtxt);
    g_free(errtxt);
    return name;
}
