#include <gtk/gtk.h>

#include "headers.h"

/**
 * Uses `g_strcmp0` to compare two guids.
 */
gint compare_guids(gpointer pa, gpointer pb) {
    const gchar *account_1 = (gchar *)pa;
    const gchar *account_2 = (gchar *)pb;
    // g_print("pa: %s, pb: %s\n", account_1, account_2);
    return g_strcmp0(account_1, account_2);
}

/**
 * Returns true if the selected account's name is one of the account codes (242, 323, 349, etc.)
 */
gboolean is_p_l_account(gchar *name, GtkTreeIter iter, Data_passer *data_passer) {
    GtkTreePath *tree_path = gtk_tree_model_get_path(GTK_TREE_MODEL(data_passer->accounts_store), &iter);
    gboolean is_pl_account = FALSE;

    /*    if ((gtk_tree_path_compare(data_passer->fixed_asset_root, tree_path) == 0) ||
           (gtk_tree_path_compare(data_passer->income_root, tree_path) == 0) ||
           (gtk_tree_path_compare(data_passer->expenses_root, tree_path) == 0)) {
           return is_p_l_account;
       } */

    for (gint i = 0; i < LENGTH_PL_ACCOUNTS_ARRAY; i++) {
        if (g_strcmp0(name, PL_ACCOUNTS_ARRAY[i]) == 0) {
            is_pl_account = TRUE;
            break;
        }
    }
    gtk_tree_path_free(tree_path);
    return is_pl_account;
}

/**
 * Finds the iter of a fixed asset in the reports tree corresponding to an iter in the accounts tree. If a match is found, returns TRUE, otherwise returns FALSE.
 */
gboolean get_report_tree_fixed_asset(GtkTreeModel *model, GtkTreeIter account_iter, GtkTreeIter *corresponding_report_iter, Data_passer *data_passer) {
    gchararray name;
    gtk_tree_model_get(model, &account_iter, 1, &name, -1);

    GtkTreeIter report_tree_iter;
    GtkTreeModel *report_model = GTK_TREE_MODEL(data_passer->reports_store);

    gtk_tree_model_get_iter_first(report_model, &report_tree_iter);

    do {
        gchararray report_account_description;
        gtk_tree_model_get(report_model, &report_tree_iter, DESCRIPTION_REPORT, &report_account_description, -1);

        if (strstr(report_account_description, name) != NULL) {
            *corresponding_report_iter = report_tree_iter;
            return TRUE;
        }
    } while (gtk_tree_model_iter_next(report_model, &report_tree_iter));
    return FALSE;
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
    data_passer->is_guid_in_reports_tree = FALSE;
    GtkTreeIter reports_root_iter;
    gboolean found_root = gtk_tree_model_get_iter_first (GTK_TREE_MODEL(data_passer->reports_store), &reports_root_iter);
   is_guid_in_reports_tree(data_passer->reports_store,reports_root_iter, guid, data_passer);

    if (data_passer->is_guid_in_reports_tree == FALSE) {
        GtkTreePath *currently_selected_path = gtk_tree_model_get_path(model, &iter);
        if (gtk_tree_path_is_descendant(currently_selected_path, data_passer->fixed_asset_root)) {
            gtk_widget_set_sensitive(data_passer->btn_add, TRUE);
            gtk_tree_path_free(currently_selected_path);
            return;
        }

        if (is_p_l_account(name, iter, data_passer) &&
            account_parent_in_report_tree(model, iter, data_passer)) {
            /* If the selected account is NOT already in the reports list, and if it
               is an account that is eligible to be in the reports list, set
               the add button's sensitivity. */
            gtk_widget_set_sensitive(data_passer->btn_add, TRUE);
            return;
        }
    }
    /* If the selected account is already in the reports list, clear the add button's sensitivity. */
    gtk_widget_set_sensitive(data_passer->btn_add, FALSE);
}

/**
 * Adds a currently selected account to the reports tree. The logic is as follows:

    * If the selection is a fixed asset, add it to the reports tree and exit.
    * Get to which fixed asset the selection belongs.
    * Determine if the selection is an expense or income.
    * Add the selection to the corresponding account in the reports tree, under expense or income as appropriate.
*/
void add_account_to_reports(GtkButton *button, gpointer user_data) {
    Data_passer *data_passer = (Data_passer *)user_data;

    GtkTreeSelection *tree_view_accounts_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data_passer->tree_view_accounts));

    gtk_tree_selection_set_mode(tree_view_accounts_selection, GTK_SELECTION_SINGLE);
    GtkTreeModel *accounts_model = gtk_tree_view_get_model(GTK_TREE_VIEW(data_passer->tree_view_accounts));
    GtkTreeIter iter_selection;
    gtk_tree_selection_get_selected(tree_view_accounts_selection, &accounts_model, &iter_selection);

    GtkTreeIter iter_parent;
    gtk_tree_model_iter_parent(accounts_model, &iter_parent, &iter_selection);
    GtkTreePath *parent_tree_path = gtk_tree_model_get_path(accounts_model, &iter_parent);

    /* If a fixed asset, add to reports model and exit. */
    if (gtk_tree_path_compare(data_passer->fixed_asset_root, parent_tree_path) == 0) {
        GtkTreeIter iter_child;
        gchararray guid;
        gtk_tree_model_get(accounts_model, &iter_selection, GUID_ACCOUNT, &guid, -1);
        gchararray description;
        gtk_tree_model_get(accounts_model, &iter_selection, DESCRIPTION_ACCOUNT, &description, -1);

        gtk_tree_store_append(data_passer->reports_store, &iter_child, NULL);
        gtk_tree_store_set(data_passer->reports_store, &iter_child, GUID_REPORT, guid, DESCRIPTION_REPORT, description, -1);
        GSList *accounts_in_reports_store;

        GtkTreeIter child_iter_income;
        gtk_tree_store_append(data_passer->reports_store, &child_iter_income, &iter_child);
        gtk_tree_store_set(data_passer->reports_store, &child_iter_income, DESCRIPTION_REPORT, REVENUE, -1);

        GtkTreeIter child_iter_expenses;
        gtk_tree_store_append(data_passer->reports_store, &child_iter_expenses, &iter_child);
        gtk_tree_store_set(data_passer->reports_store, &child_iter_expenses, DESCRIPTION_REPORT, EXPENSES, -1);

        return;
    }

    /* Determine if selection is an expense or income account. */
    GtkTreePath *tree_path_selection = gtk_tree_model_get_path(accounts_model, &iter_selection);
    gboolean is_income_account = gtk_tree_path_is_descendant(tree_path_selection, data_passer->income_root);

    /* Find the iter in the reports tree of the fixed asset. */
    GtkTreeIter corresponding_report_iter;
    if (get_report_tree_fixed_asset(accounts_model, iter_selection, &corresponding_report_iter, data_passer)) {
        /* Find the iter of the fixed asset's income or expense list */

        GtkTreeModel *reports_model = GTK_TREE_MODEL(data_passer->reports_store);
        GtkTreeIter income_expense_parent;
        gboolean found_income_expense_parent = gtk_tree_model_iter_children(reports_model, &income_expense_parent, &corresponding_report_iter);

        if (found_income_expense_parent) {
            do {
                gchararray revenue_or_expenses;
                gtk_tree_model_get(reports_model, &income_expense_parent, DESCRIPTION_REPORT, &revenue_or_expenses, -1);

                if (((strstr(revenue_or_expenses, REVENUE) != NULL) && is_income_account) ||
                    ((strstr(revenue_or_expenses, EXPENSES) != NULL) && !is_income_account)) {
                    break;
                }
            } while (gtk_tree_model_iter_next(reports_model, &income_expense_parent));
            gchararray guid;
            gtk_tree_model_get(accounts_model, &iter_selection, GUID_ACCOUNT, &guid, -1);
            gchararray description;
            gtk_tree_model_get(accounts_model, &iter_selection, DESCRIPTION_ACCOUNT, &description, -1);

            GtkTreeIter new_income_expense_entry;
            gtk_tree_store_append(data_passer->reports_store, &new_income_expense_entry, &income_expense_parent);
            gtk_tree_store_set(data_passer->reports_store, &new_income_expense_entry, GUID_REPORT, guid, DESCRIPTION_REPORT, description, -1);
        }
    }
}
