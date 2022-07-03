#include "headers.h"

void add_income_to_report_store(gpointer data, gpointer user_data) {
    Account_summary *account_summary = (Account_summary *)data;
    Iter_passer_reports *iter_passer_reports = (Iter_passer_reports *)user_data;
    GtkTreeIter child;
    gtk_tree_store_append(iter_passer_reports->reports_store, &child, &(iter_passer_reports->parent));
    g_print("%s\n", account_summary->description);
    gtk_tree_store_set(iter_passer_reports->reports_store, &child, 0, account_summary->description, -1);
    /* In following statement, structure access precedes dereferencing, so actually appending to the GSList pointer. */
    *iter_passer_reports->accounts_in_reports_store = g_slist_append(*iter_passer_reports->accounts_in_reports_store, account_summary->guid);
}

void add_property_to_store(gpointer data, gpointer user_data) {
    Property *property = (Property *)data;
    Data_passer *data_passer = (Data_passer *)user_data;

    data_passer->accounts_in_reports_store = g_slist_append(data_passer->accounts_in_reports_store, property->guid);

    GtkTreeIter parent_iter;
    gtk_tree_store_append(data_passer->reports_store, &parent_iter, NULL);
    gtk_tree_store_set(data_passer->reports_store, &parent_iter, GUID_REPORT, property->guid, DESCRIPTION_REPORT, property->description, -1);

    GtkTreeIter child_iter_income;
    gtk_tree_store_append(data_passer->reports_store, &child_iter_income, &parent_iter);
    gtk_tree_store_set(data_passer->reports_store, &child_iter_income, 0, "Revenue", -1);

    Iter_passer_reports *iter_passer_reports = g_new(Iter_passer_reports, 1);
    iter_passer_reports->reports_store = data_passer->reports_store;
    iter_passer_reports->parent = child_iter_income;
    iter_passer_reports->accounts_in_reports_store = &(data_passer->accounts_in_reports_store);
    g_slist_foreach(property->income_accounts, add_income_to_report_store, iter_passer_reports);



    GtkTreeIter child_iter_expenses;
    gtk_tree_store_append(data_passer->reports_store, &child_iter_expenses, &parent_iter);
    gtk_tree_store_set(data_passer->reports_store, &child_iter_expenses, 0, "Expenses", -1);

    iter_passer_reports->parent = child_iter_expenses;
    g_slist_foreach(property->expense_accounts, add_income_to_report_store, iter_passer_reports);
}

void read_reports_tree(Data_passer *data_passer) {
    data_passer->reports_store = gtk_tree_store_new(COLUMNS_REPORT, G_TYPE_STRING, G_TYPE_STRING);

    g_slist_foreach(data_passer->properties, add_property_to_store, data_passer);
}