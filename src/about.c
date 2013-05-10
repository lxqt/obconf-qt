#include "main.h"

void on_about_clicked()
{
    gtk_widget_show(get_widget("about_window"));
}

void on_about_close_clicked()
{
    gtk_widget_hide(get_widget("about_window"));
}

void on_about_window_delete_event()
{
    gtk_widget_hide(get_widget("about_window"));
}
