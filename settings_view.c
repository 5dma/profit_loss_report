#include <gtk/gtk.h>

#include "headers.h"

/**
 * @file settings_view.c
 * @brief Contains functions for setting up the settings dialog box.
 */

/**
 * Creates Gtk widgets in the main window.
 * @param data_passer Pointer to a Data_passer struct.
 */
GtkWidget *make_settings_dialog(Data_passer *data_passer) {
    GtkWidget *settings_dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(settings_dialog), "Settings");

    GtkWidget *label_start_date = gtk_label_new("Start date");
    GtkWidget *label_end_date = gtk_label_new("End date");
    GtkWidget *label_output_filename = gtk_label_new("Output file name");
    GtkWidget *label_sqlite_filename = gtk_label_new("Path to SQLite database");
    GtkWidget *btn_settings_save = gtk_button_new_from_icon_name("document-save", GTK_ICON_SIZE_BUTTON);
    GtkWidget *btn_settings_close = gtk_button_new_from_icon_name("stock-close", GTK_ICON_SIZE_BUTTON);

   
   // g_signal_connect(btn_settings_save, "clicked", G_CALLBACK(save_report_tree), data_passer);
 //   g_signal_connect(btn_exit, "clicked", G_CALLBACK(closeup), data_passer);



//    gtk_widget_set_tooltip_text(btn_settings_save, "Save settings");
//    gtk_widget_set_tooltip_text(btn_settings_close, "Close");

    GtkWidget *settings_grid = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(settings_grid), label_start_date, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(settings_grid), label_end_date, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(settings_grid), label_output_filename, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(settings_grid), label_sqlite_filename, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(settings_grid), btn_settings_save, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(settings_grid), btn_settings_close, 1, 4, 1, 1);
   

    gtk_grid_set_row_spacing(GTK_GRID(settings_grid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(settings_grid), 20);

    gtk_container_add(GTK_CONTAINER(settings_dialog), settings_grid);

    return settings_dialog;
}