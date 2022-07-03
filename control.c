#include <gtk/gtk.h>

#include "headers.h"

gint compare_guids(gpointer pa, gpointer pb) {
    const gchar *account_1 = (gchar *)pa;
    const gchar *account_2 = (gchar *)pb;
    g_print("pa: %s, pb: %s\n", account_1, account_2);
    return g_strcmp0(account_1, account_2);
}

gboolean is_p_l_account(gchar *name) {
    for (gint i = 0; i < LENGTH_PL_ACCOUNTS_ARRAY; i++) {
        if (g_strcmp0(name, PL_ACCOUNTS_ARRAY[i]) == 0) {
            return TRUE;
        }
    }
    return FALSE;
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
    gchararray name;
    gtk_tree_model_get(model, &iter, 1, &name, -1);

    /* Check if the currently selected account is already in the reports list. */
    GSList *already_in_reports_list = g_slist_find_custom(data_passer->accounts_in_reports_store, guid, (GCompareFunc)compare_guids);

    /* If the selected account is NOT already in the reports list, and if it
       is an account that is eligible to be in the reports list, set
       the add button's sensitivity. */
    if ((already_in_reports_list == NULL) && is_p_l_account(name)) {
        gtk_widget_set_sensitive(data_passer->btn_add, TRUE);
    } else {
        gtk_widget_set_sensitive(data_passer->btn_add, FALSE);
    }
}

void add_account_to_reports(GtkButton *button, gpointer user_data) {
    Data_passer *data_passer = (Data_passer *)user_data;

    GtkTreeSelection *tree_view_accounts_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data_passer->tree_view_accounts));

    gtk_tree_selection_set_mode(tree_view_accounts_selection, GTK_SELECTION_SINGLE);
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(data_passer->tree_view_accounts));
    GtkTreeIter iter;
    gboolean omg = gtk_tree_selection_get_selected(tree_view_accounts_selection, &model, &iter);
    gchararray guid;
    gtk_tree_model_get(model, &iter, 0, &guid, -1);
    gchararray name;
    gtk_tree_model_get(model, &iter, 1, &name, -1);

    /* If a fixed asset, add to reports model. */
    /* Get to which fixed asset the selection belongs */
    /* Get if it an expense or income. */
    /* Add to the reports model. */
    /* Add to the list of accounts in the reports tree. */
    g_print("Button clicked!\n");
}
