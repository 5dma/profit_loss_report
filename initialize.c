#include <glib-2.0/glib.h>
#include <json-glib/json-glib.h>
#include <sqlite3.h>
#include <stdio.h>

#include "headers.h"

static int property_list_builder(void *user_data, int argc, char **argv, char **azColName) {
    GSList **properties = (GSList **)user_data;

    Property *property = g_new(Property, 1);

    GSList *income = NULL;
    GSList *expenses = NULL;

    property->guid = g_strdup(argv[0]);
    property->description = g_strdup(argv[1]);
    property->income_accounts = income;
    property->expense_accounts = expenses;
    *properties = g_slist_append(*properties, property);

    return 0;
}

static int add_account_descriptions(void *user_data, int argc, char **argv, char **azColName) {
    Account_summary *account_summary = (Account_summary *)user_data;
    account_summary->description = strdup(argv[0]);
    return 0;
}

void add_accounts(Data_passer *data_passer, JsonObject *property_object, GtkTreeIter *property_iter, gint account_type) {
    GtkTreeStore *reports_store = data_passer->reports_store;
    JsonArray *account_array;
    GtkTreeIter revenue_expense_iter;
    gtk_tree_store_append(reports_store, &revenue_expense_iter, property_iter);
    if (account_type == INCOME) {
        gtk_tree_store_set(reports_store, &revenue_expense_iter, GUID_REPORT, "(blank)", DESCRIPTION_REPORT, "Revenue", -1);
        account_array = (JsonArray *)json_object_get_array_member(property_object, "income_accounts");
    } else {
        gtk_tree_store_set(reports_store, &revenue_expense_iter, GUID_REPORT, "(blank)", DESCRIPTION_REPORT, "Expenses", -1);
        account_array = (JsonArray *)json_object_get_array_member(property_object, "expense_accounts");
    }

    if (account_array != NULL) {
        guint len_accounts = json_array_get_length(account_array);
        gchararray guid;
        gchar parent_description[1000];
        GtkTreeIter account_iter;
        for (int i = 0; i < len_accounts; i++) {
            guid = strdup(json_array_get_string_element(account_array, i));
            get_parent_account_description(guid, parent_description, data_passer);
            gtk_tree_store_append(reports_store, &account_iter, &revenue_expense_iter);
            gtk_tree_store_set(reports_store, &account_iter, GUID_REPORT, guid, DESCRIPTION_REPORT, parent_description, -1);
        }
    }
}

static int retrieve_property_description(void *user_data, int argc, char **argv, char **azColName) {
    gchar *description = (gchar *)user_data;

    gsize mylength = g_strlcpy(description, argv[0], 1000);
    

    g_print("%s\n", description);
    return 0;
}

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
    return;
}

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


Data_passer *setup(GApplication *app) {
    Data_passer *data_passer = g_new(Data_passer, 1);
    data_passer->app = app;
    data_passer->accounts_store = NULL;
    data_passer->is_guid_in_reports_tree = FALSE;

    data_passer->reports_store = gtk_tree_store_new(COLUMNS_REPORT, G_TYPE_STRING, G_TYPE_STRING);

    int rc;
    char *sql;
    char *zErrMsg = 0;

    rc = sqlite3_open("/home/abba/Finances/Bookkeeping/rentals.sqlite.gnucash", &(data_passer->db));
    if (rc != SQLITE_OK) {
        printf("Can't open database: %s\n", sqlite3_errmsg(data_passer->db));
        g_free(zErrMsg);
        return (NULL);
    }

    read_properties_into_reports_store(data_passer);

    return data_passer;
}