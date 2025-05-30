#include <headers.h>
#include <sqlite3.h>

/**
 * @file accounts_files.c
 * @brief Contains functions for reading the GnuCash accounts into a tree.
 *
 */

/**
 * Gets the iters pointing to the top-level GnuCash accounts `Fixed Assets`, `Expenses`, and `Income`. The iters are saved in `data_passer->fixed_asset_root`, `data_passer->expenses_root`, and `data_passer->income_root`, respectively.
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
 * SQLite callback that returns the number of children associated with a passed parent iter. The number of children is placed in `iter_passer->number_of_children`.
 *
 * @param user_data Pointer to an Iter_passer struct.
 * @param argc Number of columns in an SQLite result.
 * @param argv Array of pointers to the results of a query.
 * @param azColName Array of pointers to strings corresponding to result column names.
 * @return Always zero.
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
 * @param iter_passer Pointer to an Iter_passer struct. The iter points to the current parent iter currently being freed.
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
 * SQLite callback that recursively builds a tree of accounts from the top-level GnuCash accounts `Fixed Assets`, `Income`, and `Expenses`.
 *
 * @param user_data Pointer to an Iter_passer struct. The iter points to the current parent iter being constructed during the recursion.
 * @param argc Number of columns in an SQLite result.
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
		iter_passer->parent = g_malloc(sizeof(GtkTreeIter));
		gtk_tree_store_append(store, iter_passer->parent, NULL);
		gtk_tree_store_set(store, iter_passer->parent, GUID_ACCOUNT, argv[0], NAME_ACCOUNT, argv[1], DESCRIPTION_ACCOUNT, argv[2], -1);
	} else {
		iter_passer->child = g_malloc(sizeof(GtkTreeIter));
		gtk_tree_store_append(store, iter_passer->child, iter_passer->parent);
		gtk_tree_store_set(store, iter_passer->child, GUID_ACCOUNT, argv[0], NAME_ACCOUNT, argv[1], DESCRIPTION_ACCOUNT, argv[2], -1);
	}

	/* Get the child accounts associated with the passed parent. */
	char sql[1000];
	g_snprintf(sql, 1000, "SELECT COUNT(*) FROM accounts WHERE parent_guid = \"%s\";", argv[0]);

	int rc;
	char *zErrMsg = 0;

	rc = sqlite3_exec(iter_passer->db, sql, has_children, iter_passer, &zErrMsg);
	if (rc != SQLITE_OK) {
		char error_message[1000];
		g_snprintf(error_message, 1000, "SQLite error: %s", sqlite3_errmsg(iter_passer->data_passer->db));
		gtk_statusbar_pop(GTK_STATUSBAR(iter_passer->data_passer->status_bar), iter_passer->data_passer->status_bar_context);
		gtk_statusbar_push(GTK_STATUSBAR(iter_passer->data_passer->status_bar), iter_passer->data_passer->status_bar_context, error_message);
		iter_passer->data_passer->error_condition = SQLITE_SELECT_FAILURE;
		sqlite3_free(zErrMsg);
		return -1;
	}
	/*
		If the current parent indeed has children, recurse into this function using each
		child as a parent.
	*/
	if (iter_passer->number_of_children > 0) {
		char child_sql[1000];
		g_snprintf(child_sql, 1000, "SELECT guid,name,description,parent_guid FROM accounts WHERE parent_guid = \"%s\";", argv[0]);

		/* Memory freed in read_accounts_tree */
		Iter_passer *iter_passer_child = g_malloc(sizeof(Iter_passer));
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
			char error_message[1000];
			g_snprintf(error_message, 1000, "SQLite error: %s", sqlite3_errmsg(iter_passer->data_passer->db));
			gtk_statusbar_pop(GTK_STATUSBAR(iter_passer->data_passer->status_bar), iter_passer->data_passer->status_bar_context);
			gtk_statusbar_push(GTK_STATUSBAR(iter_passer->data_passer->status_bar), iter_passer->data_passer->status_bar_context, error_message);
			iter_passer->data_passer->error_condition = SQLITE_SELECT_FAILURE;
			sqlite3_free(zErrMsg);
		}
	}
	return 0;
}

/**
 * Sorts the account tree. This tree effectively has two levels, and the sorting at each level is as follows:
 * - At the top level, place accounts in the following order: Fixed Assets, Income, Expenses.
 * - At the second level and below, sort by account name (e.g., Homeowners, Repairs, Taxes).
 *
 * @param model Pointer to the accounts model.
 * @param iter_a Iter pointing to the first entry in the model.
 * @param iter_b Iter pointing to the second entry in the model.
 * @param user_data `NULL` in this case.
 * @return 0, 1, or -1 depending on the value of the strings at `iter_a` and `iter_b`.
 */
gint sort_account_iter_compare_func(GtkTreeModel *model,
									GtkTreeIter *iter_a,
									GtkTreeIter *iter_b,
									gpointer user_data) {
	gchar *name_a; /* Memory freed below. */
	gchar *name_b; /* Memory freed below. */
	gint return_value = 0;

	gtk_tree_model_get(model, iter_a, NAME_ACCOUNT, &name_a, -1);
	gtk_tree_model_get(model, iter_b, NAME_ACCOUNT, &name_b, -1);

	if (((g_strcmp0(name_a, "Fixed Assets") == 0) && (g_strcmp0(name_b, "Expenses") == 0)) ||
		((g_strcmp0(name_a, "Income") == 0) && (g_strcmp0(name_b, "Expenses") == 0)) ||
		((g_strcmp0(name_a, "Income") == 0) && (g_strcmp0(name_b, "Fixed Assets") == 0))) {
		return_value = -1;

	} else {
		return_value = g_strcmp0(name_a, name_b);
	}
	g_free(name_b);
	g_free(name_a);
	return return_value;
}

/**
 * Recursively calls build_tree() to build the accounts tree from the following top-level accounts: `Fixed Assets`, `Income`, and `Expenses`. The logic is as follows:
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
 * As part of this recursion, we instantiate Iter_passer structs to manage the location of parent and child accounts. To free these structs, we place them in a `GList` as they are created. When the tree is constructed, we free the structs' memory using iter_free().
 * @param data_passer Pointer to a Data_passer struct.
 *
 */
void read_accounts_tree(Data_passer *data_passer) {
	/* The following tree store is freed in cleanup(). */
	data_passer->accounts_store = gtk_tree_store_new(COLUMNS_ACCOUNT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	/* Memory freed as part of g_list_free_full(), below.  */
	Iter_passer *iter_passer = g_malloc(sizeof(Iter_passer));
	iter_passer->db = data_passer->db;
	iter_passer->number_of_children = 0;
	iter_passer->accounts_store = data_passer->accounts_store;
	iter_passer->at_root_level = TRUE;
	iter_passer->parent = NULL;
	iter_passer->child = NULL;
	iter_passer->iters_to_be_freed = NULL;
	iter_passer->data_passer = data_passer;
	data_passer->iters_to_be_freed = iter_passer->iters_to_be_freed;
	
	iter_passer->iters_to_be_freed = g_list_append(iter_passer->iters_to_be_freed, iter_passer);


	/* Select the following accounts to be at the top level: Fixed assets, income, expenses. The forced sorting ensures the accounts tree starts in this order. */
	const gchar *sql = "SELECT guid,name,description,parent_guid FROM accounts WHERE guid IN (\"09f67b1fbae223eca818ba617edf1b3c\",  \"bde70db24873e7950e43316a246a8131\", \"420eea01b86f3681273064826ef58c7d\") ORDER BY parent_guid DESC, name DESC;";

	int rc;
	char *zErrMsg = 0;

	rc = sqlite3_exec(iter_passer->db, sql, build_tree, iter_passer, &zErrMsg);

	if (rc != SQLITE_OK) {
		char error_message[1000];
		g_snprintf(error_message, 1000, "SQLite error: %s", sqlite3_errmsg(data_passer->db));
		gtk_statusbar_pop(GTK_STATUSBAR(data_passer->status_bar), data_passer->status_bar_context);
		gtk_statusbar_push(GTK_STATUSBAR(data_passer->status_bar), data_passer->status_bar_context, error_message);
		data_passer->error_condition = SQLITE_SELECT_FAILURE;
		sqlite3_free(zErrMsg);
	}

	save_top_level_iters(data_passer);

	GList *iters_to_free = g_list_first(data_passer->iters_to_be_freed);

	g_list_free_full(iters_to_free, (GDestroyNotify)iter_free);

	/* After populating the account tree, apply sorting. */
	GtkTreeSortable *sortable;
	sortable = GTK_TREE_SORTABLE(data_passer->accounts_store);
	gtk_tree_sortable_set_sort_func(sortable, NAME_ACCOUNT, sort_account_iter_compare_func,
									GINT_TO_POINTER(NAME_ACCOUNT), NULL);
	gtk_tree_sortable_set_sort_column_id(sortable, NAME_ACCOUNT, GTK_SORT_ASCENDING);
}
