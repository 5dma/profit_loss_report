#include <stdio.h>

#include "headers.h"

/**
 * @file cleanup.c
 * @brief Contains functions that free memory before ending the application.
 *
 */

/**
 * Gtk callback fired when clicking the exit button or destroying the main window. The following members of `data_passer` are freed in other functions:
 * - output_file - Freed in save_report_tree().
 * @param btn_exit Pointer to the clicked exit button.
 * @param user_data Pointer to a Data_passer struct.
 * @return `TRUE` if the selected account's name is one of the account codes (242, 323, 349, etc.) and therefore can be included in the P&L report, `FALSE` otherwise.
 */
void cleanup(GtkWidget *window, gpointer user_data) {
    
    Data_passer *data_passer = (Data_passer *)user_data;
 //   gulong handler = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(data_passer->tree_view_reports), "cursor-changed"));
 //   g_signal_handler_disconnect(G_OBJECT(data_passer->tree_view_reports), handler);

    sqlite3_close(data_passer->db);

    g_free(data_passer->start_date);
    g_free(data_passer->end_date);

    gtk_tree_store_clear(data_passer->accounts_store);
    gtk_tree_store_clear(data_passer->reports_store);
    gtk_tree_path_free(data_passer->fixed_asset_root);
    gtk_tree_path_free(data_passer->income_root);
    gtk_tree_path_free(data_passer->expenses_root);
    g_free(data_passer);
}

/**
 * Callback that runs after clicking <b>Exit</b>. This function emits the \c destroy signal on the application window. The callback for that emitted signal is cleanup().
 * @param button_close Clicked button.
 * @param data Pointer to the data-passer structure.
 * @see cleanup()
 */
void closeup(GtkWidget *button_close, gpointer data) {
    Data_passer *data_passer = (Data_passer *)data;
    gtk_widget_destroy(data_passer->window);
}
