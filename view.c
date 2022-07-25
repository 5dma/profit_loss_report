#include <gtk/gtk.h>

#include "headers.h"

/**
 * @file view.c
 * @brief Contains functions for setting up the application's view.
 */

/* void on_drag_begin(gpointer user_data) {
    g_print("Started a drag!\n");
}

void on_drag_end(gpointer user_data) {
    g_print("on_drag_end!\n");
}
void on_drag_data_received(gpointer user_data) {
    g_print("on_drag_data_received!\n");
}
void on_drag_drop(gpointer user_data) {
    g_print("on_drag_drop!\n");
}

void on_drag_motion(gpointer user_data) {
    g_print("on_drag_motion!\n");
}
void on_drag_leave(gpointer user_data) {
    g_print("on_drag_leave!\n");
}
void on_drag_data_get(gpointer user_data) {
    g_print("on_drag_data_get!\n");
} */

/**
 * Creates Gtk widgets in the main window.
 * @param data_passer Pointer to a Data_passer struct.
 * @return pointer to the main window.
 */
GtkWidget *make_window(Data_passer *data_passer) {
    GApplication *app = data_passer->app;

    GtkWidget *window = gtk_application_window_new(GTK_APPLICATION(app));
    gtk_window_set_title(GTK_WINDOW(window), "Property P&L");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);
    data_passer->window = window;

    /* Upon destroying the application, call cleanup() to free memory in data passer. */
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(cleanup), data_passer);

    GtkWidget *lbl_accounts = gtk_label_new("Accounts");
    GtkWidget *lbl_reports = gtk_label_new("Reports");

    GtkWidget *tree_view_accounts = gtk_tree_view_new();
    GtkWidget *tree_view_reports = gtk_tree_view_new();

    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", NAME_ACCOUNT, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view_accounts), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", DESCRIPTION_REPORT, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view_reports), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Description", renderer, "text", DESCRIPTION_ACCOUNT, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view_accounts), column);

    data_passer->tree_view_accounts = tree_view_accounts;
    data_passer->tree_view_reports = tree_view_reports;

    if (data_passer->error_condition != NO_DATABASE_CONNECTION) {
        read_accounts_tree(data_passer);
    }

    gtk_tree_view_set_model(GTK_TREE_VIEW(tree_view_accounts), GTK_TREE_MODEL(data_passer->accounts_store));
    gtk_tree_view_set_model(GTK_TREE_VIEW(tree_view_reports), GTK_TREE_MODEL(data_passer->reports_store));

    GtkWidget *scrolled_window_tree_view_accounts = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window_tree_view_accounts), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window_tree_view_accounts), tree_view_accounts);
    gtk_widget_set_size_request(scrolled_window_tree_view_accounts, 300, 400);

    GtkWidget *scrolled_window_tree_view_reports = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window_tree_view_reports), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window_tree_view_reports), tree_view_reports);
    gtk_widget_set_size_request(scrolled_window_tree_view_reports, 300, 400);

    data_passer->handler = g_signal_connect(G_OBJECT(tree_view_accounts), "cursor-changed", G_CALLBACK(account_tree_cursor_changed), data_passer);
    g_signal_connect(G_OBJECT(tree_view_reports), "cursor-changed", G_CALLBACK(reports_tree_cursor_changed), data_passer);

    /*
        GtkTargetEntry target_entries[] = {
          {"text/plain", 0, STRING}};
       gtk_drag_source_set(tree_view_accounts, GDK_BUTTON1_MASK, target_entries, 1, GDK_ACTION_COPY);
      g_signal_connect(G_OBJECT(tree_view_accounts), "drag_begin", G_CALLBACK(on_drag_begin), NULL);
      g_signal_connect(G_OBJECT(tree_view_accounts), "drag_data_get", G_CALLBACK(on_drag_data_get), NULL);
      g_signal_connect(G_OBJECT(tree_view_accounts), "drag_end", G_CALLBACK(on_drag_end), NULL);

      gtk_drag_dest_set(tree_view_reports, GTK_DEST_DEFAULT_DROP, target_entries, 1, GDK_ACTION_PRIVATE);
      g_signal_connect(G_OBJECT(tree_view_reports), "drag_data_received", G_CALLBACK(on_drag_data_received), NULL);
      g_signal_connect(G_OBJECT(tree_view_reports), "drag_drop", G_CALLBACK(on_drag_drop), NULL);
      g_signal_connect(G_OBJECT(tree_view_reports), "drag_motion", G_CALLBACK(on_drag_motion), NULL);
      g_signal_connect(G_OBJECT(tree_view_reports), "drag_leave", G_CALLBACK(on_drag_leave), NULL); */

    GtkWidget *btn_save = gtk_button_new_from_icon_name("document-save", GTK_ICON_SIZE_BUTTON);
    GtkWidget *btn_revert = gtk_button_new_from_icon_name("document-revert", GTK_ICON_SIZE_BUTTON);
    GtkWidget *btn_add = gtk_button_new_from_icon_name("list-add", GTK_ICON_SIZE_BUTTON);
    GtkWidget *btn_delete = gtk_button_new_from_icon_name("list-remove", GTK_ICON_SIZE_BUTTON);
    GtkWidget *btn_go = gtk_button_new_from_icon_name("system-run", GTK_ICON_SIZE_BUTTON);
    GtkWidget *btn_exit = gtk_button_new_from_icon_name("application-exit", GTK_ICON_SIZE_BUTTON);
    GtkWidget *btn_settings = gtk_button_new_from_icon_name("gnome-settings", GTK_ICON_SIZE_BUTTON);

    gtk_widget_set_sensitive(btn_add, FALSE);
    gtk_widget_set_sensitive(btn_delete, FALSE);

    g_signal_connect(btn_save, "clicked", G_CALLBACK(save_report_tree), data_passer);
    g_signal_connect(btn_add, "clicked", G_CALLBACK(add_account_to_reports), data_passer);
    g_signal_connect(btn_delete, "clicked", G_CALLBACK(delete_account_from_reports), data_passer);
    g_signal_connect(btn_go, "clicked", G_CALLBACK(make_pl_report), data_passer);
    g_signal_connect(btn_revert, "clicked", G_CALLBACK(revert_report_tree), data_passer);
    g_signal_connect(btn_settings, "clicked", G_CALLBACK(show_settings), data_passer);
    g_signal_connect(btn_exit, "clicked", G_CALLBACK(closeup), data_passer);

    data_passer->btn_add = btn_add;
    data_passer->btn_delete = btn_delete;

    gtk_widget_set_tooltip_text(btn_save, "Save report tree");
    gtk_widget_set_tooltip_text(btn_revert, "Revert to saved report tree");
    gtk_widget_set_tooltip_text(btn_add, "Add account to report");
    gtk_widget_set_tooltip_text(btn_delete, "Remove account from report");
    gtk_widget_set_tooltip_text(btn_go, "Run report");
    gtk_widget_set_tooltip_text(btn_exit, "Exit");
    gtk_widget_set_tooltip_text(btn_settings,"Configure application");

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), lbl_accounts, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), lbl_reports, 1, 0, 7, 1);
    gtk_grid_attach(GTK_GRID(grid), scrolled_window_tree_view_accounts, 0, 1, 1, 2);
    gtk_grid_attach(GTK_GRID(grid), scrolled_window_tree_view_reports, 1, 1, 7, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_save, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_revert, 2, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_settings, 3, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_add, 4, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_delete, 5, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_go, 6, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_exit, 7, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), data_passer->status_bar, 0, 3, 8, 1);

    gtk_grid_set_row_spacing(GTK_GRID(grid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 20);

    gtk_container_add(GTK_CONTAINER(window), grid);

    return window;
}