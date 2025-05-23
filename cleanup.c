#include <headers.h>
#include <sqlite3.h>
#include <stdio.h>

/**
 * @file cleanup.c
 * @brief Contains functions that free memory before ending the application.
 *
 */

/**
 * Gtk callback from a function `g_slist_free_full` for freeing the list of PDF pages.
 * @param data Generic pointer pointing to an `HPDF_Page`.
 */
void free_pdf_pages(gpointer data) {
	g_free((HPDF_Page *)data);
}

/**
 * Gtk callback from a function `g_slist_free_full` for freeing the list of PDF bookmarks.
 * @param data Generic pointer pointing to an `HPDF_Outline`.
 */
void free_pdf_outline(gpointer data) {
	g_free((HPDF_Outline *)data);
}

/**
 * Gtk callback fired when clicking the exit button or destroying the main window. This function frees memory allocated to Data_passer.
 * @param window Pointer to the application window.
 * @param user_data Pointer to a Data_passer struct.
 * @see closeup()
 */
void cleanup(GtkWidget *window, gpointer user_data) {
	Data_passer *data_passer = (Data_passer *)user_data;
	g_signal_handler_disconnect(G_OBJECT(data_passer->tree_view_accounts), data_passer->handler);

	sqlite3_close(data_passer->db);

	if (data_passer->sqlite_path != NULL) {
		g_free(data_passer->sqlite_path);
	}
	if (data_passer->start_date != NULL) {
		g_free(data_passer->start_date);
	}
	if (data_passer->end_date != NULL) {
		g_free(data_passer->end_date);
	}

	if (data_passer->output_file_name != NULL) {
		g_free(data_passer->output_file_name);
	}


	g_date_time_unref(data_passer->current_date_time);

	g_free(data_passer->settings_passer);

	gtk_tree_store_clear(data_passer->accounts_store);
	gtk_tree_store_clear(data_passer->reports_store);
	gtk_tree_path_free(data_passer->fixed_asset_root);
	gtk_tree_path_free(data_passer->income_root);
	gtk_tree_path_free(data_passer->expenses_root);

	g_free(data_passer->page_layout);

	
	g_free(data_passer);
}

/**
 * Callback that runs after clicking the exit button. This function emits the \c destroy signal on the application window. The callback for that emitted signal is cleanup().
 * @param button_close Clicked button.
 * @param data Pointer to the data-passer structure.
 * @see cleanup()
 */
void closeup(GtkWidget *button_close, gpointer data) {
	Data_passer *data_passer = (Data_passer *)data;
	gtk_widget_destroy(data_passer->window);
}
