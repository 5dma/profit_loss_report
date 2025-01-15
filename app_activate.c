#include <gtk/gtk.h>
#include <headers.h>

/**
 * @file app_activate.c
 * @brief Contains function that starts the application.
 */

/**
 * Callback from activating the application. Calls functions to initialize the data passer, initialize the view, and show the UI.
 * @param app The GTK application.
 * @param data Pointer to user data, none in this case.
 */
void on_app_activate(GApplication *app, gpointer data) {
	Data_passer *data_passer = setup(app);

	GtkWidget *window = make_window(data_passer);

	gtk_widget_show_all(window);
}