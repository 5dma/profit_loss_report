
#include <glib-2.0/glib.h>
#include <gtk/gtk.h>
#include <sqlite3.h>

static int property_list_builder(void *user_data, int argc, char **argv, char **azColName) {
    GSList *properties = (GSList *)user_data;
    int i;
    for (i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

GSList *setup(sqlite3 *db) {
    GSList *properties;
    int rc;
    char *sql;
    char *zErrMsg = 0;
    sql =
        "SELECT guid,name,description FROM accounts WHERE guid IN \
    (\"266173f3cfe06271482dde0bc6b1e846\", \
    \"ffb3c487c5fec129d825ec954b82dd10\", \
    \"a99fd717ecac17c9ea00f27afb57ebe1\", \
    \"9a092a83bb01405f966c58952fd098cb\", \
    \"f2e672aa7081441a88c8bad21a5f78ce\", \
    \"e5f8ec46fbd34f89b1547f53382d7304\");"; /*323*/

    rc = sqlite3_exec(db, sql, property_list_builder, properties, &zErrMsg);

     if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Table created successfully\n");
    }

    return properties;
}