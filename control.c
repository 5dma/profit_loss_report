#include <gtk/gtk.h>

#include "headers.h"

gint compare_guids(gpointer pa, gpointer pb) {
    const gchar *account_1 = (gchar *) pa;
    const gchar *account_2 = (gchar *) pb;
    return g_strcmp0 (account_1, account_2);
}

void btn_report_clicked(GtkButton *button, gpointer user_data) {
    Data_passer *data_passer = (Data_passer *)user_data;
    /* Go make the report. */
    make_pl_report(data_passer);
}

void account_tree_cursor_changed(GtkTreeView *tree_view_accounts, gpointer user_data) {
    Data_passer *data_passer = (Data_passer *)user_data;
    GtkTreeSelection *tree_view_accounts_selection = gtk_tree_view_get_selection(tree_view_accounts);

    gtk_tree_selection_set_mode(tree_view_accounts_selection, GTK_SELECTION_SINGLE);
    GtkTreeModel *model = gtk_tree_view_get_model(tree_view_accounts);
    GtkTreeIter iter;
    gboolean omg = gtk_tree_selection_get_selected(tree_view_accounts_selection, &model, &iter);
    gchararray guid;
    gtk_tree_model_get(model, &iter, 0, &guid, -1);

    g_print("Changed the focus %s\n", guid);

    GSList *barf = g_slist_find_custom(data_passer->accounts_in_reports_store, "98702ef3a65ee3b807565a6f03b2f3a6", (GCompareFunc)compare_guids);
    if (barf != NULL) {
        g_print ("Found it\n");
    } else {
        g_print ("Found did not found it\n");

    }
}