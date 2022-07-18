#include <stdio.h>

#include "headers.h"

/**
 * @file cleanup.c
 * @brief Contains functions that free memory before ending the application.
 *
 */

/**
 * Gtk callback fired when clicking the exit button or destroying the main window.
 * @param btn_exit Pointer to the clicked exit button.
 * @param data_passer Pointer to a Data_passer struct.
 * @return `TRUE` if the selected account's name is one of the account codes (242, 323, 349, etc.) and therefore can be included in the P&L report, `FALSE` otherwise.
 */
void cleanup(GtkButton *btn_exit, Data_passer *data_passer) {
    sqlite3_close(data_passer->db);

    g_free(data_passer->start_date);
    g_free(data_passer->end_date);

    gtk_tree_store_clear(data_passer->accounts_store);
    gtk_tree_path_free(data_passer->fixed_asset_root);
    gtk_tree_path_free(data_passer->income_root);
    gtk_tree_path_free(data_passer->expenses_root);

    g_free(data_passer);
    g_print("Finished cleaning up\n");
}
