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
    gtk_widget_hide (data_passer->settings_passer->settings_window);
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
 * Saves the selected date in Settings_passer. The saved date is of the form `YYYY-mm-dd hh:mm:ss`.
 * 
 * This function does the following:
 * -# Extracts the selected date from the passed calendar.
 * -# Converts that date to a prefix of the form `YYYY-mm-dd`.
 * -# Determines which calender was passed, the start date or the end date.
 * -# Depending on which calendar was passed:
 *    - Appends the time ` 00:00:00` to the start date.
 *    - Appends the time ` 23:59:59` to the end date.
 * -# Saves the date in the appropriate field in Data_passer.
 * @param calendar Pointer to the clicked calendar.
 * @param user_data Pointer to a Data_passer struct.
 */
void save_date(GtkCalendar *calendar, gpointer user_data) {
    Data_passer *data_passer = (Data_passer *)user_data;

    guint year;
    guint month;
    guint day;
    gchar *date_time;

    gtk_calendar_get_date(calendar, &year, &month, &day);

    gchar *date_prefix = g_strdup_printf("%i-%02i-%02i", year, month + 1, day);
    gchar *timestamp;

    if ((gpointer)calendar == (gpointer)(data_passer->settings_passer->start_calendar)) {
        timestamp = g_strconcat(date_prefix, START_DATE_SUFFIX, NULL);
        g_free(data_passer->start_date);
        data_passer->start_date = g_strdup(timestamp);
    } else {
        timestamp = g_strconcat(date_prefix, END_DATE_SUFFIX, NULL);
        g_free(data_passer->end_date);
        data_passer->end_date = g_strdup(timestamp);
    }
    g_free(timestamp);
    g_free(date_prefix);
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
