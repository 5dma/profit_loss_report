#include <gtk/gtk.h>

#include <headers.h>

/**
 * @file settings_view.c
 * @brief Contains functions for setting up the settings dialog box.
 */

/**
 * Selects a date in the passed calendar based on the passed date string.
 * - If the date string is `NULL`, the selected date is the current date.
 * - If the date string is not `NULL`, the selected date is the passed date.
 *
 * @param widget Pointer to a calendar widget.
 * @param date Pointer to a date string of the form `YYYY-mm-dd hh:mm:ss`.
 * @param settings_passer Pointer to an variable of type Settings_passer.
 */
void set_calendar_date(GtkWidget *widget, const gchar *date, const Settings_passer *settings_passer) {
    GtkCalendar *calendar = GTK_CALENDAR(widget);
    if (date == NULL) {
        gtk_calendar_select_month(calendar, settings_passer->current_month - 1, settings_passer->current_year);
        gtk_calendar_select_day(calendar, settings_passer->current_day);
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
 * Callback fired when user clicks the checkbox to user today's date for the end date.
 * - If the user toggles ON the checkbox:
 *   -# Set the calendar's sensitivity to `FALSE`.
 *   -# Select today's date in the calendar.
 * - If the user toggles OFF the checkbox:
 *   -# Set the sensitivity to `TRUE`.
 *
 * @param button Pointer to the clicked checkbox.
 * @param user_data Pointer to a Data_passer struct.
 */
void set_today_end_date(GtkWidget *button, gpointer user_data) {
    Data_passer *data_passer = (Data_passer *)user_data;

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button))) {
        /* User wants to use today's date. */
        gtk_widget_set_sensitive(data_passer->settings_passer->end_calendar, FALSE);
        set_calendar_date(data_passer->settings_passer->end_calendar, NULL, data_passer->settings_passer);
        data_passer->end_date = NULL;

    } else {
        /* User wants to select an end date. */
        gtk_widget_set_sensitive(data_passer->settings_passer->end_calendar, TRUE);
        gtk_widget_grab_focus(data_passer->settings_passer->end_calendar);
        data_passer->settings_passer->using_today_date = FALSE;
    }
}

/**
 * Creates Gtk widgets for the settings window.
 * @param button Pointer to the clicked settings button.
 * @param user_data Pointer to a Data_passer struct.
 * @return Pointer to a GTK widget.
 */
void make_settings_dialog(GtkButton *button, gpointer user_data) {
    Data_passer *data_passer = (Data_passer *)user_data;
    /* The following window is destroyed in cleanup(). */

    GtkWidget *settings_dialog = gtk_dialog_new_with_buttons("Settings", GTK_WINDOW(data_passer->window), GTK_DIALOG_MODAL, "Close", GTK_RESPONSE_CLOSE, NULL);

    data_passer->settings_passer->settings_window = settings_dialog;

    GtkWidget *label_start_date = gtk_label_new("Start date");
    GtkWidget *label_end_date = gtk_label_new("End date");

    GtkWidget *calendar_start_date = gtk_calendar_new();
    data_passer->settings_passer->start_calendar = calendar_start_date;

    /* Sets the starting calendar to either the current date or the date in the config file. */
    set_calendar_date(calendar_start_date, data_passer->start_date, data_passer->settings_passer);

    GtkWidget *calendar_end_date = gtk_calendar_new();
    data_passer->settings_passer->end_calendar = calendar_end_date;

    /* Sets the ending calendar to either the current date or the date in the config file. */
    set_calendar_date(calendar_end_date, data_passer->end_date, data_passer->settings_passer);

    GtkWidget *use_today_end_date = gtk_check_button_new_with_label("Use today's date");

    if (data_passer->settings_passer->using_today_date == TRUE) {
        /* If we are using today's date, then display the checkbox as checked and disable the ending calendar. */
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_today_end_date), TRUE);
        gtk_widget_set_sensitive(data_passer->settings_passer->end_calendar, FALSE);
    } else {
        /* If we are NOT using today's date, then display the checkbox as clear. */
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_today_end_date), FALSE);
    }

    GtkWidget *label_output_filename = gtk_label_new("Output file name");
    GtkWidget *text_output_filename = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(text_output_filename), data_passer->output_file_name);
    data_passer->settings_passer->text_output_filename = text_output_filename;

    GtkWidget *btn_output_filename = gtk_button_new_with_label("Choose...");

    GtkWidget *label_sqlite_filename = gtk_label_new("Path to SQLite database");
    GtkWidget *text_sqlite_filename = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(text_sqlite_filename), data_passer->sqlite_path);

    GtkWidget *btn_sqlite_filename = gtk_button_new_with_label("Choose...");

    g_signal_connect(btn_output_filename, "clicked", G_CALLBACK(get_output_filename), data_passer);
    g_signal_connect(btn_sqlite_filename, "clicked", G_CALLBACK(get_sqlite_filename), data_passer);
    g_signal_connect(calendar_end_date, "day-selected", G_CALLBACK(save_date), data_passer);
    g_signal_connect(calendar_start_date, "day-selected", G_CALLBACK(save_date), data_passer);
    g_signal_connect(use_today_end_date, "toggled", G_CALLBACK(set_today_end_date), data_passer);

    GtkWidget *settings_grid = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(settings_grid), label_start_date, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(settings_grid), calendar_start_date, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(settings_grid), label_end_date, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(settings_grid), calendar_end_date, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(settings_grid), use_today_end_date, 2, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(settings_grid), label_output_filename, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(settings_grid), text_output_filename, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(settings_grid), btn_output_filename, 2, 2, 1, 1);

    gtk_grid_attach(GTK_GRID(settings_grid), label_sqlite_filename, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(settings_grid), text_sqlite_filename, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(settings_grid), btn_sqlite_filename, 2, 3, 1, 1);

    gtk_grid_set_row_spacing(GTK_GRID(settings_grid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(settings_grid), 20);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(settings_dialog));

    gtk_container_add(GTK_CONTAINER(content_area), settings_grid);

    gtk_window_set_transient_for(GTK_WINDOW(settings_dialog), GTK_WINDOW(data_passer->window));
    gtk_widget_show_all(settings_dialog);
    gtk_dialog_run(GTK_DIALOG(settings_dialog));
    gtk_widget_destroy(settings_dialog);
}