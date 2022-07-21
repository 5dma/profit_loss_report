#include <gtk/gtk.h>

#include "headers.h"

/**
 * @file display_settings_control.c
 * @brief Contains functions for displaying and updating the application's configuration.
 *
 */

/**
 * Gtk callback fired when the delete button is clicked. This function removes the currently selected row in the reports tree view from the corresponding tree store.
 * @param button Pointer to the clicked delete button.
 * @param user_data Pointer to a Data_passer struct.
 */
void show_settings(GtkButton *button, gpointer user_data) {
    Data_passer *data_passer = (Data_passer *)user_data;
    g_print("Started settings\n");

    gtk_widget_show_all(data_passer->settings_dialog);

    g_print("Ended settings\n");
}
