#include <sqlite3.h>

#include "headers.h"

static int has_children(void *user_data, int argc, char **argv, char **azColName) {
    Data_passer *data_passer = (Data_passer *)user_data;

    data_passer->number_of_children = atoi(argv[0]);

    //    g_print("The number of children is %d\n", data_passer->number_of_children);
    return (0);
}

static int build_tree(void *user_data, int argc, char **argv, char **azColName) {
    Data_passer *data_passer = (Data_passer *)user_data;
    GtkTreeStore *store = data_passer->accounts_store;

    if (g_strcmp0 (  argv[0], "3b7d34a311409d76e3b83c7a575b02e1") == 0) {
        gtk_tree_store_append(store, &(data_passer->parent), NULL);
        gtk_tree_store_set(store, &(data_passer->parent), GUID, argv[0], NAME, argv[1], DESCRIPTION, argv[2], -1);
    } else {
        gtk_tree_store_append(store, &(data_passer->child), &(data_passer->parent));
        gtk_tree_store_set(store, &(data_passer->child), GUID, argv[0], NAME, argv[1], DESCRIPTION, argv[2], -1);
    }

    char sql[1000];
    gint num_bytes = g_snprintf(sql, 1000, "SELECT COUNT(*) FROM accounts WHERE parent_guid = \"%s\";", argv[0]);
    //  g_print("%s\n", sql);
    int rc;
    char *zErrMsg = 0;

    rc = sqlite3_exec(data_passer->db, sql, has_children, data_passer, &zErrMsg);

    if (data_passer->number_of_children > 0) {
        char child_sql[1000];
        gint num_bytes = g_snprintf(child_sql, 1000, "SELECT guid,name,description FROM accounts WHERE parent_guid = \"%s\";", argv[0]);
        //   g_print("%s\n", child_sql);
        rc = sqlite3_exec(data_passer->db, child_sql, build_tree, data_passer, &zErrMsg);

        if (rc != SQLITE_OK) {
            g_print("SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        } else {
            //          g_print("Everything is good\n");
        }
    }
    return 0;
}

void read_accounts_tree(Data_passer *data_passer) {
    /* Retreieve the root account from the GnuCash database */
    data_passer->accounts_store = gtk_tree_store_new(COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    const gchar *sql = "SELECT guid,name,description FROM accounts WHERE guid = \"3b7d34a311409d76e3b83c7a575b02e1\";";
    int rc;
    char *zErrMsg = 0;

    rc = sqlite3_exec(data_passer->db, sql, build_tree, data_passer, &zErrMsg);

    if (rc != SQLITE_OK) {
        g_print("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        //     g_print("Everything is good\n");
    }
}