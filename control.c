#include <gtk/gtk.h>

#include "headers.h"

gint compare_guids(gpointer pa, gpointer pb) {
    const gchar *account_1 = (gchar *)pa;
    const gchar *account_2 = (gchar *)pb;
    g_print("pa: %s, pb: %s\n", account_1, account_2);
    return g_strcmp0(account_1, account_2);
}

gboolean is_p_l_account(gchar *name, GtkTreeIter iter, Data_passer *data_passer) {
    GtkTreePath *tree_path = gtk_tree_model_get_path(GTK_TREE_MODEL(data_passer->accounts_store), &iter);
    gboolean is_pl_account = FALSE;

    for (gint i = 0; i < LENGTH_PL_ACCOUNTS_ARRAY; i++) {
        if ((g_strcmp0(name, PL_ACCOUNTS_ARRAY[i]) == 0) &&
            (gtk_tree_path_compare(data_passer->fixed_asset_root, tree_path) != 0) &&
            (gtk_tree_path_compare(data_passer->income_root, tree_path) != 0) &&
            (gtk_tree_path_compare(data_passer->expenses_root, tree_path) != 0)) {
            is_pl_account = TRUE;
            break;
        }
    }
    gtk_tree_path_free(tree_path);
    return is_pl_account;
}

gboolean account_parent_in_report_tree(GtkTreeModel *accounts_model, GtkTreeIter accounts_iter, Data_passer *data_passer) {
    /* Check if under income or expense.
        Get name (323, 242, 349, etc.), which indicates the fixed asset
         Check if fixed asset is in top level of reports tree
     */

 
    GtkTreePath *accounts_path_selection = gtk_tree_model_get_path(accounts_model, &accounts_iter);
    gboolean is_income_account = gtk_tree_path_is_descendant(accounts_path_selection, data_passer->income_root);
    gboolean is_expense_account = gtk_tree_path_is_descendant(accounts_path_selection, data_passer->expenses_root);

    if (is_income_account || is_expense_account) {
        /* Get the name */
        gchararray name;
        gtk_tree_model_get(accounts_model, &accounts_iter, NAME_ACCOUNT, &name, -1);
       
        /* See if the name is in one of the top-level report accounts. */
        GtkTreeIter report_tree_iter;
        GtkTreeModel *report_model = GTK_TREE_MODEL(data_passer->reports_store);

        gtk_tree_model_get_iter_first(report_model, &report_tree_iter);

        do {
            gchararray report_account_description;
            gtk_tree_model_get(report_model, &report_tree_iter, DESCRIPTION_REPORT, &report_account_description, -1);
            if (strstr(report_account_description, name) != NULL) {
                return TRUE;
            }
        } while (gtk_tree_model_iter_next(report_model, &report_tree_iter));
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

    if ((already_in_reports_list == NULL) &&
        is_p_l_account(name, iter, data_passer) &&
        account_parent_in_report_tree(model, iter, data_passer)) {
        /* If the selected account is NOT already in the reports list, and if it
           is an account that is eligible to be in the reports list, set
           the add button's sensitivity. */
        gtk_widget_set_sensitive(data_passer->btn_add, TRUE);
    } else {
        /* If the selected account is already in the reports list, clear the add button's sensitivity. */
        gtk_widget_set_sensitive(data_passer->btn_add, FALSE);
    }
}

void add_account_to_reports(GtkButton *button, gpointer user_data) {
    Data_passer *data_passer = (Data_passer *)user_data;

    GtkTreeSelection *tree_view_accounts_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data_passer->tree_view_accounts));

    gtk_tree_selection_set_mode(tree_view_accounts_selection, GTK_SELECTION_SINGLE);
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(data_passer->tree_view_accounts));
    GtkTreeIter iter_selection;
    gboolean omg = gtk_tree_selection_get_selected(tree_view_accounts_selection, &model, &iter_selection);

    /* Get to which fixed asset the selection belongs */
    /* Get if it an expense or income. */
    /* Add selected account to the reports model. */
    /* Add to the list of accounts in the reports tree. */

    GtkTreeIter iter_parent;
    gtk_tree_model_iter_parent(model, &iter_parent, &iter_selection);
    GtkTreePath *parent_tree_path = gtk_tree_model_get_path(model, &iter_parent);

    /* If a fixed asset, add to reports model and exit. */
    if (gtk_tree_path_compare(data_passer->fixed_asset_root, parent_tree_path) == 0) {
        GtkTreeIter iter_child;
        gchararray guid;
        gtk_tree_model_get(model, &iter_selection, GUID_ACCOUNT, &guid, -1);
        gchararray description;
        gtk_tree_model_get(model, &iter_selection, DESCRIPTION_ACCOUNT, &description, -1);

        gtk_tree_store_append(data_passer->reports_store, &iter_child, NULL);
        gtk_tree_store_set(data_passer->reports_store, &iter_child, GUID_REPORT, guid, DESCRIPTION_REPORT, description, -1);
        GSList *accounts_in_reports_store;

        data_passer->accounts_in_reports_store = g_slist_append(data_passer->accounts_in_reports_store, guid);

        return;
    }

    /* Determine if selection is an expense or income account. */
    GtkTreePath *tree_path_selection = gtk_tree_model_get_path(model, &iter_selection);
    gboolean is_income_account = gtk_tree_path_is_descendant(tree_path_selection, data_passer->income_root);

    g_print("Button clicked!\n");
}
