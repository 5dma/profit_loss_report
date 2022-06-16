
#include <glib-2.0/glib.h>
#include <sqlite3.h>

#include "headers.h"

static int property_list_builder(void *user_data, int argc, char **argv, char **azColName) {
    
    GSList **properties = (GSList **)user_data;

    Property *property = g_new(Property, 1);

    GSList *income = NULL;
    GSList *expenses = NULL;

    property->guid = g_strdup(argv[0]);
    property->description = g_strdup(argv[1]);
    property->income = income;
    property->expenses = expenses;
    *properties = g_slist_append(*properties, property);

    /*     int i;
        for (i = 0; i < argc; i++) {
            printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        }
        printf("\n");
     */
    return 0;
}

GSList *setup(sqlite3 *db) {
    GSList *properties = NULL;
    int rc;
    char *sql;
    char *zErrMsg = 0;
    sql =
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
    }

    return properties;
}