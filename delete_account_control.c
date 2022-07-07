#include <gtk/gtk.h>

#include "headers.h"

void delete_account_from_reports(GtkButton *button, gpointer user_data) {
        Data_passer *data_passer = (Data_passer *)user_data;

    GtkTreeSelection *tree_view_reports_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data_passer->tree_view_reports));

    gtk_tree_selection_set_mode(tree_view_reports_selection, GTK_SELECTION_SINGLE);
    GtkTreeModel *reports_model = gtk_tree_view_get_model(GTK_TREE_VIEW(data_passer->tree_view_reports));
    GtkTreeIter iter_selection;
    gtk_tree_selection_get_selected(tree_view_reports_selection, &reports_model, &iter_selection);

    gboolean omg = gtk_tree_store_remove(data_passer->reports_store, &iter_selection);
    g_print("Row trashed\n");
}

void reports_tree_cursor_changed(GtkTreeView *tree_view_accounts, gpointer user_data) {
     Data_passer *data_passer = (Data_passer *)user_data;
       gtk_widget_set_sensitive(data_passer->btn_delete, TRUE);
    g_print("reports cursor changed\n");
}