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
    return 0;
}

/**
 * Issues an sqlite command to retrieve an account's description corresponding to a passed guid.
 *
 * @param guid guid for which we are looking up a description.
 * @param description Retrieved description.
 * @param user_data  Pointer to a Data_passer struct.
 */
void get_account_description(const gchar *guid, gchar *description, Data_passer *data_passer) {
    int rc;
    char sql[1000];
    char *zErrMsg = 0;

    gint num_bytes = g_snprintf(sql, 1000, SELECT_DESCRIPTION_FROM_ACCOUNT, guid);
    rc = sqlite3_exec(data_passer->db, sql, retrieve_property_description, description, &zErrMsg);
    if (rc != SQLITE_OK) {
        char error_message[1000];
        gint num_bytes = g_snprintf(error_message, 1000, "SQLite error: %s", sqlite3_errmsg(data_passer->db));
        gtk_statusbar_pop(GTK_STATUSBAR(data_passer->status_bar), data_passer->status_bar_context);
        gtk_statusbar_push(GTK_STATUSBAR(data_passer->status_bar), data_passer->status_bar_context, error_message);
        data_passer->error_condition = SQLITE_SELECT_FAILURE;
        sqlite3_free(zErrMsg);
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
    if (rc != SQLITE_OK) {
        char error_message[1000];
        gint num_bytes = g_snprintf(error_message, 1000, "SQLite error: %s", sqlite3_errmsg(data_passer->db));
        gtk_statusbar_pop(GTK_STATUSBAR(data_passer->status_bar), data_passer->status_bar_context);
        gtk_statusbar_push(GTK_STATUSBAR(data_passer->status_bar), data_passer->status_bar_context, error_message);
        data_passer->error_condition = SQLITE_SELECT_FAILURE;
        sqlite3_free(zErrMsg);
    }
    return;
}

/**
 * An initializing function that does the following:
 * 
 * - Opens the JSON file in ~/.profit_loss/accounts.json
 * - Creates a GTK statusbar and saves the pointer in data_passer.
 * - Saves the pointer to the root JSON object in data_passer.
 *
 * @param data_passer  Pointer to a Data_passer struct.
 */
void read_sqlite_filename_json_object(Data_passer *data_passer) {
    gchar *input_file = g_build_filename(g_get_home_dir(), ".profit_loss/accounts.json", NULL);
    gboolean input_file_exists = g_file_test(input_file, G_FILE_TEST_EXISTS);
    JsonParser *parser;

    GtkWidget *status_bar = gtk_statusbar_new();
    data_passer->status_bar = status_bar;
    data_passer->status_bar_context = gtk_statusbar_get_context_id(GTK_STATUSBAR(status_bar), "informational");
    gtk_statusbar_push(GTK_STATUSBAR(data_passer->status_bar), data_passer->status_bar_context, "Ready");
    if (input_file_exists) {
        GError *error = NULL;

        JsonNode *root;

        /* Reference count decremented at end of this function. */
        parser = json_parser_new();
        json_parser_load_from_file(parser, input_file, &error);

        if (error) {
            char error_message[1000];
            gint num_bytes = g_snprintf(error_message, 1000, "Unable to parse `%s': %s\n", input_file, error->message);
            gtk_statusbar_pop(GTK_STATUSBAR(data_passer->status_bar), data_passer->status_bar_context);
            gtk_statusbar_push(GTK_STATUSBAR(data_passer->status_bar), data_passer->status_bar_context, "No properties in report tree, no save performed");
            data_passer->error_condition = JSON_PROCESSING_FAILURE;
            g_error_free(error);
        } else {
            JsonNode *root = json_parser_get_root(parser);
            data_passer->root_obj = json_node_get_object(root);

            /* Pretty sure no need to free following string as it is part of the root_obj instance. */
            const gchar *sqlite_path = json_object_get_string_member(data_passer->root_obj, "sqlite_file");
            if (sqlite_path != NULL) {
                data_passer->sqlite_path = g_strdup(sqlite_path);
            } else {
                data_passer->sqlite_path = NULL;
            }
        }

    } else {
        gtk_statusbar_pop(GTK_STATUSBAR(data_passer->status_bar), data_passer->status_bar_context);
        gtk_statusbar_push(GTK_STATUSBAR(data_passer->status_bar), data_passer->status_bar_context, "Input file containing accounts does not exist.");
        data_passer->error_condition = JSON_PROCESSING_FAILURE;
    }
    g_free(input_file);
    //g_object_unref(parser); /* Possible memory leak here. If I unref, I get a seg fault. */
}

/**
 * Initializes the data passer, and opens the connection to the database.
 * @param app The GTK application.
 * @return Pointer to the data passer.
 * @see read_sqlite_filename_json_object()
 */
Data_passer *setup(GApplication *app) {
    /* Memory freed in cleanup(). */
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
    data_passer->handler = 0;
    data_passer->error_condition = NONE;
    data_passer->root_obj = NULL;
    /* The following line is here instead of in read_properties_into_reports_store(), because that function is used to load data into the tree store, not to instantiate the tree view. */
    data_passer->reports_store = gtk_tree_store_new(COLUMNS_REPORT, G_TYPE_STRING, G_TYPE_STRING);

    Settings_passer *settings_passer = g_new(Settings_passer, 1);
    data_passer->settings_passer = settings_passer;

    read_sqlite_filename_json_object(data_passer);

    /* Go read the JSON file containing list of accounts in the P&L report. */
    if (data_passer->error_condition != JSON_PROCESSING_FAILURE) {
        int rc;
        char *sql;
        char *zErrMsg = 0;
        rc = sqlite3_open_v2(data_passer->sqlite_path, &(data_passer->db), SQLITE_OPEN_READONLY, NULL);
        if (rc != SQLITE_OK) {
            char error_message[1000];

            gint num_bytes = g_snprintf(error_message, 1000, "SQLite error: %s", sqlite3_errmsg(data_passer->db));

            gtk_statusbar_pop(GTK_STATUSBAR(data_passer->status_bar), data_passer->status_bar_context);
            gtk_statusbar_push(GTK_STATUSBAR(data_passer->status_bar), data_passer->status_bar_context, error_message);
            data_passer->error_condition = NO_DATABASE_CONNECTION;
            sqlite3_free(zErrMsg);
        } else {
            read_properties_into_reports_store(data_passer);
        }
    }
    /* Open read-only connection to database, save handle in data_passer. */

    return data_passer;
}