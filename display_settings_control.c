#include <gtk/gtk.h>

#include "headers.h"

/**
 * @file display_settings_control.c
 * @brief Contains functions for displaying and updating the application's configuration.
 *
 */

/**
 * Gtk callback fired when the settings close button is clicked. This function destroys the settings window.
 * @param button Pointer to the clicked close button.
 * @param user_data Pointer to a Data_passer struct.
 */
void close_settings(GtkButton *button, gpointer user_data) {
    Data_passer *data_passer = (Data_passer *)user_data;
    gtk_widget_destroy(data_passer->settings_passer->settings_window);
}

/**
 * Gtk callback fired when the the Choose button corresponding to the output file is clicked. This function displays a GTK file chooser. If the user selects a file, it is saved into Data_passer.
 * @param button Pointer to the clicked choose button.
 * @param user_data Pointer to a Data_passer struct.
 */
void get_output_filename(GtkButton *button, gpointer user_data) {
    Data_passer *data_passer = (Data_passer *)user_data;
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Output file name", GTK_WINDOW(data_passer->settings_passer->settings_window), GTK_FILE_CHOOSER_ACTION_SAVE, "Cancel", GTK_RESPONSE_CANCEL,
        "Choose", GTK_RESPONSE_OK, NULL);
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));

    gchar *filename;
    if (result == GTK_RESPONSE_OK) {
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        data_passer->output_file_name = strdup(filename);
        gtk_entry_set_text(GTK_ENTRY(data_passer->settings_passer->text_output_filename), filename);
    }
    gtk_widget_destroy(dialog);
    g_free(filename);
}

/**
 * Gtk callback fired when the the Choose button corresponding to the SQLite file is clicked. This function displays a GTK file chooser. If the user selects a file, it is saved into Data_passer.
 * @param button Pointer to the clicked choose button.
 * @param user_data Pointer to a Data_passer struct.
 */

void get_sqlite_filename(GtkButton *button, gpointer user_data) {
    Data_passer *data_passer = (Data_passer *)user_data;
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "SQLite file name", GTK_WINDOW(data_passer->settings_passer->settings_window), GTK_FILE_CHOOSER_ACTION_SAVE, "Cancel", GTK_RESPONSE_CANCEL,
        "Choose", GTK_RESPONSE_OK, NULL);
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));

    gchar *filename;
    if (result == GTK_RESPONSE_OK) {
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        data_passer->sqlite_path = strdup(filename);
        gtk_entry_set_text(GTK_ENTRY(data_passer->settings_passer->text_sqlite_filename), filename);
    }
    gtk_widget_destroy(dialog);
    g_free(filename);
}

/**
 * Gtk callback fired when the settings button is clicked. This function displays the settings window.
 * @param button Pointer to the clicked delete button.
 * @param user_data Pointer to a Data_passer struct.
 */
void show_settings(GtkButton *button, gpointer user_data) {
    Data_passer *data_passer = (Data_passer *)user_data;
    gtk_widget_show_all(data_passer->settings_dialog);
}
