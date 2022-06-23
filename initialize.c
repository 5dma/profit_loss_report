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

void add_accounts(Property *property, JsonObject *property_object, gint account_type) {
    JsonArray *account_array;

    if (account_type == INCOME) {
        account_array = (JsonArray *)json_object_get_array_member(property_object, "income_accounts");
    } else {
        account_array = (JsonArray *)json_object_get_array_member(property_object, "expense_accounts");
    }

    GSList *accounts = NULL;

    if (account_array != NULL) {
        guint len_accounts = json_array_get_length(account_array);

        for (int i = 0; i < len_accounts; i++) {
            Account_summary *account_summary = g_new(Account_summary, 1);
            account_summary->guid = strdup(json_array_get_string_element(account_array, i));
            account_summary->description = NULL;
            account_summary->subtotal = 0;
            accounts = g_slist_append(accounts, account_summary);
        }
    }
    if (account_type == INCOME) {
        property->income_accounts = accounts;
    } else {
        property->expense_accounts = accounts;
    }
}

static int retrieve_property_description(void *user_data, int argc, char **argv, char **azColName) {
    Property *property = (Property *)user_data;

    property->description = g_strdup(argv[0]);

    return 0;
}

void add_property_descriptions(gpointer data, gpointer user_data) {
    Property *property = (Property *)data;
    Data_passer *data_passer = (Data_passer *)user_data;

    int rc;
    char sql[1000];
    char *zErrMsg = 0;
  /*   sql =
        "SELECT description FROM accounts WHERE guid = "9a092a83bb01405f966c58952fd098cb"; */

    gchar *format_string = "SELECT description FROM accounts WHERE guid = \"%s\";";

    gint num_bytes = g_snprintf (sql, 1000, format_string, property->guid);

    rc = sqlite3_exec(data_passer->db, sql, retrieve_property_description, property, &zErrMsg);

    if (rc != SQLITE_OK) {
        g_print("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        g_print("Table created successfully\n");
    }

}


Data_passer *setup() {
    Data_passer *data_passer = g_new(Data_passer, 1);
    data_passer->properties = NULL;
    int rc;
    char *sql;
    char *zErrMsg = 0;

    rc = sqlite3_open("/home/abba/Finances/Bookkeeping/rentals.sqlite.gnucash", &(data_passer->db));
    if (rc) {
        printf("Can't open database: %s\n", sqlite3_errmsg(data_passer->db));
        g_free(zErrMsg);
        return (NULL);
    }

      data_passer->output_file = fopen("/tmp/property_pl.html","w");

    if (data_passer->output_file == NULL){
        g_print("Cannot create the report, exiting\n");
        return (NULL);
    }

    /* Memory is freed at end of this function */
    gchar *input_file = g_build_filename(g_get_home_dir(), ".profit_loss/accounts.json", NULL);
    gboolean input_file_exists = g_file_test(input_file, G_FILE_TEST_EXISTS);
    if (input_file_exists) {
        GError *error = NULL;
        JsonParser *parser;
        JsonNode *root;
        parser = json_parser_new();
        json_parser_load_from_file(parser, input_file, &error);
        if (error) {
            g_print("Unable to parse `%s': %s\n", input_file, error->message);
            g_error_free(error);
        } else {
            JsonNode *root = json_parser_get_root(parser);
            JsonObject *root_obj = json_node_get_object(root);

            data_passer->default_tz = g_time_zone_new_local();

            /* Pretty sure no need to free following string as it is a const. */
            const gchar *start_date_string = json_object_get_string_member(root_obj, "start_date");
            if (start_date_string != NULL) {
                data_passer->start_date = g_strdup(start_date_string);
            } else {
                data_passer->start_date = NULL;
            }
            /* Pretty sure no need to free following string as it is a const. */
            const gchar *end_date_string = json_object_get_string_member(root_obj, "end_date");
            if (end_date_string != NULL) {
                data_passer->end_date = g_strdup(end_date_string);
            } else {
                data_passer->end_date = NULL;
            }
            JsonArray *property_array = (JsonArray *)json_object_get_array_member(root_obj, "properties");
            guint len_properties = json_array_get_length(property_array);
            for (int i = 0; i < len_properties; i++) {
                JsonObject *property_object = json_array_get_object_element(property_array, i);
                Property *property = g_new(Property, 1);
                property->guid = g_strdup(json_object_get_string_member(property_object, "guid"));
                property->description = NULL;

                add_accounts(property, property_object, INCOME);
                add_accounts(property, property_object, EXPENSE);

                data_passer->properties = g_slist_append(data_passer->properties, property);
            }
        }
        g_object_unref(parser);
    } else {
        g_print("Input file does not exist.\n");
    }
    g_free(input_file);

    /*    sql =
           "SELECT guid,description FROM accounts WHERE guid IN \
       (\"266173f3cfe06271482dde0bc6b1e846\", \
       \"ffb3c487c5fec129d825ec954b82dd10\", \
       \"a99fd717ecac17c9ea00f27afb57ebe1\", \
       \"9a092a83bb01405f966c58952fd098cb\", \
       \"f2e672aa7081441a88c8bad21a5f78ce\", \
       \"e5f8ec46fbd34f89b1547f53382d7304\");";

       rc = sqlite3_exec(db, sql, property_list_builder, &properties, &zErrMsg);

       if (rc != SQLITE_OK) {
           g_print("SQL error: %s\n", zErrMsg);
           sqlite3_free(zErrMsg);
       } else {
           g_print("Table created successfully\n");
       }

       gint i = g_slist_length(properties);
       g_print("After it's all over,  length: %d\n", i);

       for (gint i = 0; i < g_slist_length(properties); i++) {
           gpointer *barf = g_slist_nth_data(properties, i);
           Property *omg = (Property *)barf;

           g_print("guid %s, description %s\n", omg->guid, omg->description);
       } */

        g_slist_foreach(data_passer->properties,add_property_descriptions,data_passer);

    return data_passer;
}