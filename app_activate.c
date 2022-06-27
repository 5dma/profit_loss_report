#include <gtk/gtk.h>

#include "headers.h"

void on_app_activate(GApplication *app, gpointer data) {
    Data_passer *data_passer = setup(app);

    GtkWidget *window = make_window(data_passer);


    gtk_widget_show_all(GTK_WIDGET(window));
}