#include <gtk/gtk.h>

#include "headers.h"

/**
 * @file settings_view.c
 * @brief Contains functions for setting up the settings dialog box.
 */

void set_calendar_date(GtkWidget *widget, const gchar *date, const gint current_year, const gint current_month, const gint current_day) {
    GtkCalendar *calendar = GTK_CALENDAR(widget);
    if (date == NULL) {
         gtk_calendar_select_month(calendar, current_month, current_year);
        gtk_calendar_select_day(calendar, current_day);
    } else {
        gchar *start_year_string = g_utf8_substring(date, 0, 4);
        gchar *start_month_string = g_utf8_substring(date, 5, 7);
        gchar *start_day_string = g_utf8_substring(date, 8, 10);

        gint start_year = atoi(start_year_string);
        gint start_month = atoi(start_month_string) - 1;
        gint start_day = atoi(start_day_string);

        gtk_calendar_select_month(calendar, start_month, start_year);
        gtk_calendar_select_day(calendar, start_day);

        g_free(start_year_string);
        g_free(start_month_string);
        g_free(start_day_string);

    }
}

/**
 * Creates Gtk widgets for the settings window.
 * @param data_passer Pointer to a Data_passer struct.
 * @return Pointer to a GTK widget.
 */
GtkWidget *make_settings_dialog(Data_passer *data_passer) {
    /* The following window is destroyed in cleanup(). */
    GtkWidget *settings_dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(settings_dialog), "Settings");
    gtk_window_set_modal(GTK_WINDOW(settings_dialog), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(settings_dialog), GTK_WINDOW(data_passer->window));

    data_passer->settings_passer->settings_window = settings_dialog;

    GtkWidget *label_start_date = gtk_label_new("Start date");
    GtkWidget *label_end_date = gtk_label_new("End date");

    GtkWidget *calendar_start_date = gtk_calendar_new();
    data_passer->settings_passer->start_calendar = calendar_start_date;

    /* gint values representing the current year, month day. Used to set the start and end calendars when
       no value is in the config file. */
    const gint current_year = g_date_time_get_year(data_passer->current_date_time);
    const gint current_month = g_date_time_get_month(data_passer->current_date_time);
    const gint current_day = g_date_time_get_day_of_month(data_passer->current_date_time);

    /* Sets the starting calendar to either the current date or the date in the config file. */
    set_calendar_date(calendar_start_date, data_passer->start_date, current_year, current_month, current_day);

    GtkWidget *calendar_end_date = gtk_calendar_new();
    data_passer->settings_passer->end_calendar = calendar_end_date;

    /* Sets the ending calendar to either the current date or the date in the config file. */
    set_calendar_date(calendar_end_date, data_passer->end_date, current_year, current_month, current_day);
    

    GtkWidget *label_output_filename = gtk_label_new("Output file name");
    GtkWidget *text_output_filename = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(text_output_filename), data_passer->output_file_name);
    data_passer->settings_passer->text_output_filename = text_output_filename;

    GtkWidget *btn_output_filename = gtk_button_new_with_label("Choose...");

    GtkWidget *label_sqlite_filename = gtk_label_new("Path to SQLite database");
    GtkWidget *text_sqlite_filename = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(text_sqlite_filename), data_passer->sqlite_path);

    GtkWidget *btn_sqlite_filename = gtk_button_new_with_label("Choose...");

    GtkWidget *btn_settings_close = gtk_button_new_from_icon_name("dialog-close", GTK_ICON_SIZE_BUTTON);

    g_signal_connect(btn_output_filename, "clicked", G_CALLBACK(get_output_filename), data_passer);
    g_signal_connect(btn_sqlite_filename, "clicked", G_CALLBACK(get_sqlite_filename), data_passer);
    g_signal_connect(btn_settings_close, "clicked", G_CALLBACK(close_settings), data_passer);
    g_signal_connect(calendar_end_date, "day-selected", G_CALLBACK(save_date), data_passer);
    g_signal_connect(calendar_start_date, "day-selected", G_CALLBACK(save_date), data_passer);

    gtk_widget_set_tooltip_text(btn_settings_close, "Close. Any changes you make are automatically saved.");

    GtkWidget *settings_grid = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(settings_grid), label_start_date, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(settings_grid), calendar_start_date, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(settings_grid), label_end_date, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(settings_grid), calendar_end_date, 1, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(settings_grid), label_output_filename, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(settings_grid), text_output_filename, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(settings_grid), btn_output_filename, 2, 2, 1, 1);

    gtk_grid_attach(GTK_GRID(settings_grid), label_sqlite_filename, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(settings_grid), text_sqlite_filename, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(settings_grid), btn_sqlite_filename, 2, 3, 1, 1);

    gtk_grid_attach(GTK_GRID(settings_grid), btn_settings_close, 2, 4, 1, 1);

    gtk_grid_set_row_spacing(GTK_GRID(settings_grid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(settings_grid), 20);

    gtk_container_add(GTK_CONTAINER(settings_dialog), settings_grid);

    return settings_dialog;
}