#include "headers.h"

void add_income_to_report_store(gpointer data, gpointer user_data) {
    Account_summary *account_summary = (Account_summary *)data;
    Iter_passer_reports *iter_passer_reports = (Iter_passer_reports *)user_data;
    GtkTreeIter child;
    gtk_tree_store_append(iter_passer_reports->reports_store, &child, &(iter_passer_reports->parent));
    // g_print("%s\n", account_summary->description);
    gtk_tree_store_set(iter_passer_reports->reports_store, &child, GUID_REPORT, account_summary->guid, DESCRIPTION_REPORT, account_summary->description, -1);
    
}

void add_property_to_store(gpointer data, gpointer user_data) {
    Property *property = (Property *)data;
    Data_passer *data_passer = (Data_passer *)user_data;

    GtkTreeIter parent_iter;
    gtk_tree_store_append(data_passer->reports_store, &parent_iter, NULL);
    gtk_tree_store_set(data_passer->reports_store, &parent_iter, GUID_REPORT, property->guid, DESCRIPTION_REPORT, property->description, -1);

    GtkTreeIter child_iter_income;
    gtk_tree_store_append(data_passer->reports_store, &child_iter_income, &parent_iter);
    gtk_tree_store_set(data_passer->reports_store, &child_iter_income, DESCRIPTION_REPORT, REVENUE, -1);

    Iter_passer_reports *iter_passer_reports = g_new(Iter_passer_reports, 1);
    iter_passer_reports->reports_store = data_passer->reports_store;
    iter_passer_reports->parent = child_iter_income;
    g_slist_foreach(property->income_accounts, add_income_to_report_store, iter_passer_reports);

    GtkTreeIter child_iter_expenses;
    gtk_tree_store_append(data_passer->reports_store, &child_iter_expenses, &parent_iter);
    gtk_tree_store_set(data_passer->reports_store, &child_iter_expenses, DESCRIPTION_REPORT, EXPENSES, -1);

    iter_passer_reports->parent = child_iter_expenses;
    g_slist_foreach(property->expense_accounts, add_income_to_report_store, iter_passer_reports);
}

void read_reports_tree(Data_passer *data_passer) {
    data_passer->reports_store = gtk_tree_store_new(COLUMNS_REPORT, G_TYPE_STRING, G_TYPE_STRING);

    g_slist_foreach(data_passer->properties, add_property_to_store, data_passer);
}

/**
 * Performs a depth-first-search for a GUID inside the reports store. The logic is as follows.
 *
 * -# Receive the store, root iter, guid we are looking for, and the data passer.
 * -# If a match was found (`data_passer->is_guid_in_reports_tree == TRUE`), return.
 * -# Retrieve the guid at the current iter.
 * -# If the retrieved guid matches the target guid:
 *    -# A match was found  (`data_passer->is_guid_in_reports_tree = TRUE`).
 *    -# Return.
 * -# If the current iter has children:
 *    -# Get the first child iter.
 *    -# Recurse with  store, child iter, guid we are looking for, and the data passer
 * -# If the current iter has a sibling:
 *    -# Get the sibling iter.
 *    -# Recurse with  store, sibling iter, guid we are looking for, and the data passer
 * -# Return.
 */
void is_guid_in_reports_tree(GtkTreeStore *reports_store, GtkTreeIter current_iter, char *guid, Data_passer *data_passer) {
    if (data_passer->is_guid_in_reports_tree == TRUE) {
        return;
    }

    gchararray candidate_guid;
    gtk_tree_model_get(GTK_TREE_MODEL(reports_store), &current_iter, GUID_REPORT, &candidate_guid, -1);

    if (g_strcmp0(candidate_guid, guid) == 0) {
        data_passer->is_guid_in_reports_tree = TRUE;
        return;
    }
    GtkTreeIter child_iter;
    gboolean current_iter_has_children = gtk_tree_model_iter_children(GTK_TREE_MODEL(reports_store), &child_iter, &current_iter);
    if (current_iter_has_children == TRUE) {
        is_guid_in_reports_tree(reports_store, child_iter, guid, data_passer);
    }

    GtkTreeIter sibling_iter;
    gboolean current_iter_has_sibling = gtk_tree_model_iter_next (GTK_TREE_MODEL(reports_store), &current_iter);
    if (current_iter_has_sibling == TRUE) {
        is_guid_in_reports_tree(reports_store, current_iter, guid, data_passer);
    }

  return;

}