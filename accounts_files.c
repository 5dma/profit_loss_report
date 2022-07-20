#include <sqlite3.h>

#include "headers.h"

/**
 * @file accounts_files.c
 * @brief Contains functions for reading the GnuCash accounts into a tree.
 *
 */

/**
 * Gets the iters pointing to the top-level GnuCash accounts Fixed Assets, Expenses, and Income. The iters are saved in `data_passer->fixed_asset_root`, and `data_passer->expenses_root`, and `data_passer->income_root`, respectively.
 *
 * @param data_passer Pointer to a Data_passer struct.
 */
void save_top_level_iters(Data_passer *data_passer) {
    GtkTreeIter iter;
    gtk_tree_model_get_iter_first(GTK_TREE_MODEL(data_passer->accounts_store), &iter);

    /*
    09f67b1fbae223eca818ba617edf1b3c - Fixed Assets
    420eea01b86f3681273064826ef58c7d - Expenses
    bde70db24873e7950e43316a246a8131 - Income
    */
    do {
        gchar *guid; /* Memory freed below. */
        gtk_tree_model_get(GTK_TREE_MODEL(data_passer->accounts_store), &iter, 0, &guid, -1);

        /* Is this a fixed asset iter? */
        if (g_strcmp0(guid, "09f67b1fbae223eca818ba617edf1b3c") == 0) {
            /* data_passer->fixed_asset_root freed in cleanup(). */
            data_passer->fixed_asset_root = gtk_tree_model_get_path(GTK_TREE_MODEL(data_passer->accounts_store), &iter);
            continue;
        }
        /* Is this an expenses iter? */
        if (g_strcmp0(guid, "420eea01b86f3681273064826ef58c7d") == 0) {
            /* data_passer->expenses_root freed in cleanup(). */
            data_passer->expenses_root = gtk_tree_model_get_path(GTK_TREE_MODEL(data_passer->accounts_store), &iter);
            continue;
        }
        /* Is this an expenses iter? */
        if (g_strcmp0(guid, "bde70db24873e7950e43316a246a8131") == 0) {
            /* data_passer->income_root freed in cleanup(). */
            data_passer->income_root = gtk_tree_model_get_path(GTK_TREE_MODEL(data_passer->accounts_store), &iter);
        }
        g_free(guid);

    } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(data_passer->accounts_store), &iter) != FALSE);
}

/**
 * Sqlite callback that returns the number of children associated with a passed parent iter. The number of children is placed in `iter_passer->number_of_children`.
 *
 * @param user_data Pointer to a Iter_passer struct.
 * @param argc Number of columns in sqlite result.
 * @param argv Array of pointers to the results of a query.
 * @param azColName Array of pointers to strings corresponding to result column names.
 * @see [One-Step Query Execution Interface](https://www.sqlite.org/c3ref/exec.html)
 */
static int has_children(void *user_data, int argc, char **argv, char **azColName) {
    Iter_passer *iter_passer = (Iter_passer *)user_data;
    iter_passer->number_of_children = atoi(argv[0]);
    return 0;
}

/**
 * GList callback that frees the contents of a passed Iter_passer.
 *
 * @param iter_passer Pointer to a Iter_passer struct. The iter points to the current parent iter we are constructing during the recursion.
 * @see read_accounts_tree()
 */
void iter_free(Iter_passer *iter_passer) {
    iter_passer->db = NULL;
    iter_passer->accounts_store = NULL;
    if (iter_passer->parent != NULL) {
        gtk_tree_iter_free(iter_passer->parent);
    }
    if (iter_passer->child != NULL) {
        gtk_tree_iter_free(iter_passer->child);
    }
    iter_passer->iters_to_be_freed = NULL;
}

/**
 * Sqlite callback that recursively builds a tree of accounts from the top-level GnuCash accounts Fixed Assets, Income, and Expenses.
 *
 * @param user_data Pointer to a Iter_passer struct. The iter points to the current parent iter we are constructing during the recursion.
 * @param argc Number of columns in sqlite result.
 * @param argv Array of pointers to the results of a query.
 * @param azColName Array of pointers to strings corresponding to result column names.
 * @return 0 if the recursion is successful.
 * @see [One-Step Query Execution Interface](https://www.sqlite.org/c3ref/exec.html)
 */
static int build_tree(void *user_data, int argc, char **argv, char **azColName) {
    Iter_passer *iter_passer = (Iter_passer *)user_data;
    GtkTreeStore *store = iter_passer->accounts_store;

    /*
        If we are at the root level (one of the three root accounts),
        add a row at that root level and store the guid, name, and description.
    */
    if (iter_passer->at_root_level == TRUE) {
        iter_passer->parent = g_new(GtkTreeIter, 1);
        gtk_tree_store_append(store, iter_passer->parent, NULL);
        gtk_tree_store_set(store, iter_passer->parent, GUID_ACCOUNT, argv[0], NAME_ACCOUNT, argv[1], DESCRIPTION_ACCOUNT, argv[2], -1);
    } else {
        iter_passer->child = g_new(GtkTreeIter, 1);
        gtk_tree_store_append(store, iter_passer->child, iter_passer->parent);
        gtk_tree_store_set(store, iter_passer->child, GUID_ACCOUNT, argv[0], NAME_ACCOUNT, argv[1], DESCRIPTION_ACCOUNT, argv[2], -1);
    }

    /* Get the child accounts associated with the passed parent. */
    char sql[1000];
    gint num_bytes = g_snprintf(sql, 1000, "SELECT COUNT(*) FROM accounts WHERE parent_guid = \"%s\";", argv[0]);

    int rc;
    char *zErrMsg = 0;

    rc = sqlite3_exec(iter_passer->db, sql, has_children, iter_passer, &zErrMsg);
    if (rc != SQLITE_OK) {
        g_print("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }
    /*
        If the current parent indeed has children, recurse into this function using each
        child as a parent.
    */
    if (iter_passer->number_of_children > 0) {
        char child_sql[1000];
        gint num_bytes = g_snprintf(child_sql, 1000, "SELECT guid,name,description,parent_guid FROM accounts WHERE parent_guid = \"%s\";", argv[0]);

        /* Memory freed in read_accounts_tree */
        Iter_passer *iter_passer_child = g_new(Iter_passer, 1);
        iter_passer_child->db = iter_passer->db;
        iter_passer_child->accounts_store = iter_passer->accounts_store;
        iter_passer_child->number_of_children = 0;
        iter_passer_child->parent = NULL;
        iter_passer_child->child = NULL;

        if (iter_passer->at_root_level == TRUE) {
            iter_passer_child->parent = gtk_tree_iter_copy(iter_passer->parent);
        } else {
            iter_passer_child->parent = gtk_tree_iter_copy(iter_passer->child);
        }
        iter_passer_child->at_root_level = FALSE;
        /* Add the current Iter_passer to the list of Iter_passers that will eventually be freed. */
        iter_passer_child->iters_to_be_freed = iter_passer->iters_to_be_freed;
        iter_passer_child->iters_to_be_freed = g_list_append(iter_passer_child->iters_to_be_freed, iter_passer_child);

        rc = sqlite3_exec(iter_passer_child->db, child_sql, build_tree, iter_passer_child, &zErrMsg);

        if (rc != SQLITE_OK) {
            g_print("SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        } else {
            //          g_print("Everything is good\n");
        }
    }
    return 0;
}

/**
 * Selects the following accounts to be at the top level of the accounts tree: Fixed assets, Income, Expenses, and then recursively calls build_tree() to build the accounts tree. The logic is as follows:
 *
 * -# Retrieve the three accounts at the top level.
 * -# For each such account:
 *    -# Call build_tree() with the parent account as `NULL` and the child account as the current account.
 *    -# Within build_tree(), are we at the top level?
 *       - If so, add the current account to the top level of the tree.
 *       - If not, add the current account as a child to the current parent.
 *       - Does the current account have children?
 *         - If so, loop through each child, calling build_tree() with the parent account as the current account and the child account as the current child account.
 *         - If not, return.
 *
 * As part of this recursion, we instantiate Iter_passer structs to manage the locaton of parent and child accounts. To free these structs, we place them in a `GList` as they are created. When the tree is constructed, we free the structs' memory using iter_free().
 * @param data_passer Pointer to a Data_passer struct.
 *
 */
void read_accounts_tree(Data_passer *data_passer) {
    /* The following tree store is freed in cleanup(). */
    data_passer->accounts_store = gtk_tree_store_new(COLUMNS_ACCOUNT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    /* Memory freed as part of g_list_free_full(), below.  */
    Iter_passer *iter_passer = g_new(Iter_passer, 1);
    iter_passer->db = data_passer->db;
    iter_passer->number_of_children = 0;
    iter_passer->accounts_store = data_passer->accounts_store;
    iter_passer->at_root_level = TRUE;
    iter_passer->parent = NULL;
    iter_passer->child = NULL;
    iter_passer->iters_to_be_freed = NULL;
    iter_passer->iters_to_be_freed = g_list_append(iter_passer->iters_to_be_freed, iter_passer);

    data_passer->iters_to_be_freed = iter_passer->iters_to_be_freed;

    /* Select the following accounts to be at the top level: Fixed assets, income, expenses. The forced sorting ensures the accounts tree starts in this order. */
    const gchar *sql = "SELECT guid,name,description,parent_guid FROM accounts WHERE guid IN (\"09f67b1fbae223eca818ba617edf1b3c\",  \"bde70db24873e7950e43316a246a8131\", \"420eea01b86f3681273064826ef58c7d\") ORDER BY parent_guid DESC, name DESC;";

    int rc;
    char *zErrMsg = 0;

    rc = sqlite3_exec(iter_passer->db, sql, build_tree, iter_passer, &zErrMsg);

    if (rc != SQLITE_OK) {
        g_print("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        //     g_print("Everything is good\n");09f67b1fbae223eca818ba617edf1b3c
    }

    save_top_level_iters(data_passer);

    GList *iters_to_free = g_list_first(data_passer->iters_to_be_freed);

    g_list_free_full(iters_to_free, (GDestroyNotify)iter_free);
}
