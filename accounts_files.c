#include <sqlite3.h>

#include "headers.h"


void save_top_level_iters(Data_passer *data_passer) {

}


static int has_children(void *user_data, int argc, char **argv, char **azColName) {
    Iter_passer *iter_passer = (Iter_passer *)user_data;

    iter_passer->number_of_children = atoi(argv[0]);

    //    g_print("The number of children is %d\n", data_passer->number_of_children);
    return (0);
}

static int build_tree(void *user_data, int argc, char **argv, char **azColName) {
    Iter_passer *iter_passer = (Iter_passer *)user_data;
    GtkTreeStore *store = iter_passer->accounts_store;

    if (iter_passer->at_root_level == TRUE) {
        gtk_tree_store_append(store, &(iter_passer->parent), NULL);
        gtk_tree_store_set(store, &(iter_passer->parent), GUID_ACCOUNT, argv[0], NAME_ACCOUNT, argv[1], DESCRIPTION_ACCOUNT, argv[2], -1);
    } else {
        gtk_tree_store_append(store, &(iter_passer->child), &(iter_passer->parent));
        gtk_tree_store_set(store, &(iter_passer->child), GUID_ACCOUNT, argv[0], NAME_ACCOUNT, argv[1], DESCRIPTION_ACCOUNT, argv[2], -1);
    }

    char sql[1000];
    gint num_bytes = g_snprintf(sql, 1000, "SELECT COUNT(*) FROM accounts WHERE parent_guid = \"%s\";", argv[0]);
    int rc;
    char *zErrMsg = 0;

    rc = sqlite3_exec(iter_passer->db, sql, has_children, iter_passer, &zErrMsg);

    if (iter_passer->number_of_children > 0) {
        char child_sql[1000];
        gint num_bytes = g_snprintf(child_sql, 1000, "SELECT guid,name,description,parent_guid FROM accounts WHERE parent_guid = \"%s\";", argv[0]);

        /* Memory freed below */
        Iter_passer *iter_passer_child = g_new(Iter_passer, 1);
        iter_passer_child->db = iter_passer->db;
        iter_passer_child->accounts_store = iter_passer->accounts_store;
        iter_passer_child->number_of_children = 0;
        if (iter_passer->at_root_level == TRUE) {
            iter_passer_child->parent = iter_passer->parent;
        } else {
            iter_passer_child->parent = iter_passer->child;
        }
        iter_passer_child->at_root_level = FALSE;

        rc = sqlite3_exec(iter_passer_child->db, child_sql, build_tree, iter_passer_child, &zErrMsg);

        if (rc != SQLITE_OK) {
            g_print("SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        } else {
            //          g_print("Everything is good\n");
        }
        g_free(iter_passer_child);
    }
    return 0;
}

/**
     Retrieves the root account from the GnuCash database into a `GtkTreeStore`.
*/
void read_accounts_tree(Data_passer *data_passer) {
    data_passer->accounts_store = gtk_tree_store_new(COLUMNS_ACCOUNT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    /* Memory freed below */
    Iter_passer *iter_passer = g_new(Iter_passer, 1);
    iter_passer->db = data_passer->db;
    iter_passer->number_of_children = 0;
    iter_passer->accounts_store = data_passer->accounts_store;
    iter_passer->at_root_level = TRUE;

    /* Select all top-level accounts (children of ROOT) except the following:
     * Liabilities
     * Imbalance-USD
     * Orphan-USD accounts. */
    /*     const gchar *sql = "SELECT guid,name,description,parent_guid FROM accounts WHERE parent_guid = \"3b7d34a311409d76e3b83c7a575b02e1\" AND guid NOT IN (\"894f0ff8c1ea9e1da084b8a50e396427\",\"34e3fc202d62305f5e9cfdeff2732cef\", \"4c34d0b684e591b31042b8dabd52c20f\");";
     */

    /* Select the following accounts to be at the top level: Fixed assets, income, expenses. The forced sorting ensures the accounts tree starts in this order. */
    const gchar *sql = "SELECT guid,name,description,parent_guid FROM accounts WHERE guid IN (\"09f67b1fbae223eca818ba617edf1b3c\",  \"bde70db24873e7950e43316a246a8131\", \"420eea01b86f3681273064826ef58c7d\") ORDER BY parent_guid DESC, name DESC;";

    int rc;
    char *zErrMsg = 0;

    rc = sqlite3_exec(iter_passer->db, sql, build_tree, iter_passer, &zErrMsg);

    if (rc != SQLITE_OK) {
        g_print("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        //     g_print("Everything is good\n");
    }

    save_top_level_iters(data_passer);

    g_free(iter_passer);
}