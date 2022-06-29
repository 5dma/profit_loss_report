#include <sqlite3.h>

#include "headers.h"

static int has_children(void *user_data, int argc, char **argv, char **azColName) {
    Iter_passer *iter_passer = (Iter_passer *)user_data;

    iter_passer->number_of_children = atoi(argv[0]);

    //    g_print("The number of children is %d\n", data_passer->number_of_children);
    return (0);
}

static int build_tree(void *user_data, int argc, char **argv, char **azColName) {
    Iter_passer *iter_passer = (Iter_passer *)user_data;
    GtkTreeStore *store = iter_passer->accounts_store;

    g_print("Processing results for guid %s %s\n", argv[0], argv[1]);

    if (iter_passer->tree_level == ROOT) {
        gtk_tree_store_append(store, &(iter_passer->parent), NULL);
        gtk_tree_store_set(store, &(iter_passer->parent), GUID, argv[0], NAME, argv[1], DESCRIPTION, argv[2], -1);
        iter_passer->tree_level = LEVEL_1;
    } else {
        gtk_tree_store_append(store, &(iter_passer->child), &(iter_passer->parent));
        gtk_tree_store_set(store, &(iter_passer->child), GUID, argv[0], NAME, argv[1], DESCRIPTION, argv[2], -1);
    }

    char sql[1000];
    gint num_bytes = g_snprintf(sql, 1000, "SELECT COUNT(*) FROM accounts WHERE parent_guid = \"%s\";", argv[0]);
    g_print("%s\n", sql);
    int rc;
    char *zErrMsg = 0;

    rc = sqlite3_exec(iter_passer->db, sql, has_children, iter_passer, &zErrMsg);

    if (iter_passer->number_of_children > 0) {
        char child_sql[1000];
        gint num_bytes = g_snprintf(child_sql, 1000, "SELECT guid,name,description,parent_guid FROM accounts WHERE parent_guid = \"%s\";", argv[0]);

        g_print("%s\n", child_sql);
        /* Memory freed below */
        Iter_passer *iter_passer_child = g_new(Iter_passer, 1);
        iter_passer_child->db = iter_passer->db;
        iter_passer_child->accounts_store = iter_passer->accounts_store;
        iter_passer_child->number_of_children = 0;
        if (iter_passer->tree_level == LEVEL_1) {
            iter_passer_child->parent = iter_passer->parent;
        } else {
            iter_passer_child->parent = iter_passer->child;
        }
        iter_passer_child->tree_level = BELOW_LEVEL_1;
    

        //   g_print("%s\n", child_sql);
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
    data_passer->accounts_store = gtk_tree_store_new(COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    /* Memory freed below */
    Iter_passer *iter_passer = g_new(Iter_passer, 1);
    iter_passer->db = data_passer->db;
    iter_passer->number_of_children = 0;
    iter_passer->accounts_store = data_passer->accounts_store;
    iter_passer->tree_level = ROOT;

    const gchar *sql = "SELECT guid,name,description,parent_guid FROM accounts WHERE parent_guid = \"3b7d34a311409d76e3b83c7a575b02e1\";";
    int rc;
    char *zErrMsg = 0;

    rc = sqlite3_exec(iter_passer->db, sql, build_tree, iter_passer, &zErrMsg);

    if (rc != SQLITE_OK) {
        g_print("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        //     g_print("Everything is good\n");
    }

    g_free(iter_passer);
    /*    gtk_tree_store_append( data_passer->accounts_store, &(iter_passer->parent), NULL);
   gtk_tree_store_set( data_passer->accounts_store, &(iter_passer->parent), GUID,"12345", NAME, "12345", DESCRIPTION, "GAG", -1);

   gtk_tree_store_append(data_passer->accounts_store, &(iter_passer->child), &(iter_passer->parent));
   gtk_tree_store_set(data_passer->accounts_store, &(iter_passer->child),GUID,"54321", NAME, "54321", DESCRIPTION, "GAG", -1);

   gtk_tree_store_append(data_passer->accounts_store, &(iter_passer->child), &(iter_passer->parent));
   gtk_tree_store_set(data_passer->accounts_store, &(iter_passer->child),GUID,"55555", NAME, "55555", DESCRIPTION, "GAG", -1);


 gtk_tree_store_append( data_passer->accounts_store, &(iter_passer->parent), NULL);
   gtk_tree_store_set( data_passer->accounts_store, &(iter_passer->parent), GUID,"67890", NAME, "67890", DESCRIPTION, "GAG", -1);

   gtk_tree_store_append(data_passer->accounts_store, &(iter_passer->child), &(iter_passer->parent));
   gtk_tree_store_set(data_passer->accounts_store, &(iter_passer->child),GUID,"54321", NAME, "54321", DESCRIPTION, "GAG", -1);

   gtk_tree_store_append(data_passer->accounts_store, &(iter_passer->child), &(iter_passer->parent));
   gtk_tree_store_set(data_passer->accounts_store, &(iter_passer->child),GUID,"55555", NAME, "55555", DESCRIPTION, "GAG", -1); */
}