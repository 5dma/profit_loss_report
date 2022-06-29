#include <gtk/gtk.h>

#include "headers.h"

GtkWidget *make_window(Data_passer *data_passer) {
    GApplication *app = data_passer->app;

    GtkWidget *window = gtk_application_window_new(GTK_APPLICATION(app));
    gtk_window_set_title(GTK_WINDOW(window), "Property P&L");
    gtk_window_set_default_size (GTK_WINDOW(window), 500, 500);

    GtkWidget *lbl_accounts = gtk_label_new("Accounts");
    GtkWidget *lbl_reports = gtk_label_new("Reports");

    GtkWidget *tree_view_accounts = gtk_tree_view_new();
    GtkWidget *tree_view_reports = gtk_tree_view_new();

    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", NAME, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view_accounts), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", NAME, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view_reports), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Description", renderer, "text", DESCRIPTION, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view_accounts), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Description", renderer, "text", DESCRIPTION, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view_reports), column);

    read_accounts_tree(data_passer);

    gtk_tree_view_set_model(GTK_TREE_VIEW(tree_view_accounts), GTK_TREE_MODEL(data_passer->accounts_store));
    gtk_tree_view_set_model(GTK_TREE_VIEW(tree_view_reports), GTK_TREE_MODEL(data_passer->accounts_store));


    GtkWidget *scrolled_window_tree_view_accounts = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window_tree_view_accounts), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window_tree_view_accounts), tree_view_accounts);
    gtk_widget_set_size_request(scrolled_window_tree_view_accounts, 300, 400); 

    GtkWidget *scrolled_window_tree_view_reports = gtk_scrolled_window_new(NULL, NULL);
     gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window_tree_view_reports), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window_tree_view_reports), tree_view_reports);
    gtk_widget_set_size_request(scrolled_window_tree_view_reports, 300, 400); 



    GtkWidget *btn_save = gtk_button_new_from_icon_name("document-save", GTK_ICON_SIZE_BUTTON);
    GtkWidget *btn_revert = gtk_button_new_from_icon_name("document-revert", GTK_ICON_SIZE_BUTTON);
    GtkWidget *btn_delete = gtk_button_new_from_icon_name("edit-delete", GTK_ICON_SIZE_BUTTON);
    GtkWidget *btn_go = gtk_button_new_from_icon_name("system-run", GTK_ICON_SIZE_BUTTON);
    GtkWidget *btn_exit = gtk_button_new_from_icon_name("application-exit", GTK_ICON_SIZE_BUTTON);

    gtk_widget_set_tooltip_text(btn_save, "Save");
    gtk_widget_set_tooltip_text(btn_revert, "Revert");
    gtk_widget_set_tooltip_text(btn_delete, "Delete");
    gtk_widget_set_tooltip_text(btn_go, "Run");
    gtk_widget_set_tooltip_text(btn_exit, "Exit");

    GtkWidget *status_bar = gtk_statusbar_new();

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), lbl_accounts, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), lbl_reports, 1, 0, 4, 1);
    gtk_grid_attach(GTK_GRID(grid), scrolled_window_tree_view_accounts, 0, 1, 1, 2);
    gtk_grid_attach(GTK_GRID(grid), scrolled_window_tree_view_reports, 1, 1, 5, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_save, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_revert, 2, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_delete, 3, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_go, 4, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_exit, 5, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), status_bar, 0, 3, 6, 1);

    gtk_grid_set_row_spacing(GTK_GRID(grid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 20);

    gtk_container_add(GTK_CONTAINER(window), grid);
    return window;
    /* Upon destroying the application, free memory in data passer. */
    g_signal_connect(window, "destroy", G_CALLBACK(cleanup), data_passer);
}