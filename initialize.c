#include <glib-2.0/glib.h>
#include <json-glib/json-glib.h>
#include <sqlite3.h>
#include <stdio.h>
#include "headers.h"

/**
 * @file initialize.c
 * @brief Contains function that starts the application.
 */

/**
 * Adds accounts to the report tree, under the passed property and under expenses or income.
 * 
 * @param data_passer Pointer to a Data_passer struct.
 * @param property_object Pointer to the current property for which we are adding income or expense accounts.
 * @param property_iter Pointer to the property's iter in the reports tree store.
 * @param account_type `INCOME` if adding income accounts, `EXPENSE` if adding expense accounts.
 */
void add_accounts(Data_passer *data_passer, JsonObject *property_object, GtkTreeIter *property_iter, gint account_type) {
    GtkTreeStore *reports_store = data_passer->reports_store;
    JsonArray *account_array;
    GtkTreeIter revenue_expense_iter;
    gtk_tree_store_append(reports_store, &revenue_expense_iter, property_iter);

    /* Add a literal `Revenue` or `Expenses` record in the reports tree under the current property. */
    if (account_type == INCOME) {
        gtk_tree_store_set(reports_store, &revenue_expense_iter, GUID_REPORT, "(blank)", DESCRIPTION_REPORT, "Revenue", -1);
        account_array = (JsonArray *)json_object_get_array_member(property_object, "income_accounts");
    } else {
        gtk_tree_store_set(reports_store, &revenue_expense_iter, GUID_REPORT, "(blank)", DESCRIPTION_REPORT, "Expenses", -1);
        account_array = (JsonArray *)json_object_get_array_member(property_object, "expense_accounts");
    }

    /* If the JSON file included income or expense accounts to add, add them under the `Revenue` or `Expenses` record we just created. */
    if (account_array != NULL) {
        guint len_accounts = json_array_get_length(account_array);
        gchar parent_description[1000];
        GtkTreeIter account_iter;
        for (int i = 0; i < len_accounts; i++) {
            gchar *guid = strdup(json_array_get_string_element(account_array, i));
            get_parent_account_description(guid, parent_description, data_passer);
            gtk_tree_store_append(reports_store, &account_iter, &revenue_expense_iter);
            gtk_tree_store_set(reports_store, &account_iter, GUID_REPORT, guid, DESCRIPTION_REPORT, parent_description, -1);
            g_free(guid);
        }
    }
}

/**
 * Sqlite callback that retrieves a property's description. The function places the description in the passed `user_data` pointer.
 * 
 * @param user_data Pointer to `gchar`.
 * @param argc Number of columns in sqlite result.
 * @param argv Array of pointers to the results of a query.
 * @param azColName Array of pointers to strings corresponding to result column names.
 * @return 0.
 * @see [One-Step Query Execution Interface](https://www.sqlite.org/c3ref/exec.html)
 * @see get_account_description()
*/
static int retrieve_property_description(void *user_data, int argc, char **argv, char **azColName) {
    gchar *description = (gchar *)user_data;

    gsize mylength = g_strlcpy(description, argv[0], 1000);

    g_print("%s\n", description);
    return 0;
}

/**
 * Issues an sqlite command to retrieve an account's description corresponding to a passed guid.
 * 
 * @param guid guid for which we are looking up a description.
 * @param description Retrieved description.
 * @param user_data  Pointer to a Data_passer struct.
*/
void get_account_description(gchar *guid, gchar *description, gpointer user_data) {
    Data_passer *data_passer = (Data_passer *)user_data;

    int rc;
    char sql[1000];
    char *zErrMsg = 0;

    gint num_bytes = g_snprintf(sql, 1000, SELECT_DESCRIPTION_FROM_ACCOUNT, guid);
    rc = sqlite3_exec(data_passer->db, sql, retrieve_property_description, description, &zErrMsg);
    g_print("%s\n", description);
    if (rc != SQLITE_OK) {
        g_print("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
    }
}

/**
 * Issues an sqlite command to retrieve, for a passed guid, the parent account's description.
 * 
 * @param guid guid for which we are looking up its parent node's description.
 * @param description Retrieved description.
 * @param user_data  Pointer to a Data_passer struct.
*/
void get_parent_account_description(gchar *guid, gchar *description, gpointer user_data) {
    Data_passer *data_passer = (Data_passer *)user_data;

    int rc;
    char sql[1000];
    char *zErrMsg = 0;

    gint num_bytes = g_snprintf(sql, 1000, SELECT_DESCRIPTION_FROM_PARENT_ACCOUNT, guid);
    rc = sqlite3_exec(data_passer->db, sql, retrieve_property_description, description, &zErrMsg);
    g_print("%s\n", description);
    if (rc != SQLITE_OK) {
        g_print("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
    }
    return;
}

/**
 * Initializes the data passer, and opens the connection to the database.
 * @param app The GTK application.
 * @return Pointer to the data passer.
 */
Data_passer *setup(GApplication *app) {
    Data_passer *data_passer = g_new(Data_passer, 1);

    data_passer->db = NULL;
    data_passer->start_date = NULL;
    data_passer->end_date = NULL;
    data_passer->output_file = NULL;
    data_passer->total_revenues = 0;
    data_passer->total_expenses = 0;
    data_passer->subtotaling_revenues = FALSE;
    data_passer->app = app;
    data_passer->accounts_store = NULL;
    data_passer->tree_view_accounts = NULL;
    data_passer->tree_view_reports = NULL;
    data_passer->btn_add = NULL;
    data_passer->btn_delete = NULL;
    data_passer->fixed_asset_root = NULL;
    data_passer->income_root = NULL;
    data_passer->expenses_root = NULL;
    data_passer->is_guid_in_reports_tree = FALSE;
    /* The following line is here instead of in read_properties_into_reports_store(), because that function is used to load data into the tree store, not to instantiate the tree view. */
    data_passer->reports_store = gtk_tree_store_new(COLUMNS_REPORT, G_TYPE_STRING, G_TYPE_STRING);

    /* Open connection to database, save handle in data_passer. */
    int rc;
    char *sql;
    char *zErrMsg = 0;
    rc = sqlite3_open("/home/abba/Finances/Bookkeeping/rentals.sqlite.gnucash", &(data_passer->db));
    if (rc != SQLITE_OK) {
        printf("Can't open database: %s\n", sqlite3_errmsg(data_passer->db));
        g_free(zErrMsg);
        return (NULL);
    }

    /* Go read the JSON file containing list of accounts in the P&L report. */
    read_properties_into_reports_store(data_passer);

    return data_passer;
}