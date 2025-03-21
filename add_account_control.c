#include <gtk/gtk.h>
#include <headers.h>

/**
 * @file add_account_control.c
 * @brief Contains functions for adding an account to the reports tree.
 *
 */

/**
 * Determines if a passed account name is `242`, `323`, `349`, or any of the accounts in the PL_ACCOUNTS_ARRAY.
 * @param name
 * @param iter
 * @param data_passer Pointer to a Data_passer struct.
 * @return `TRUE` if the selected account's name is one of the account codes (`242`, `323`, `349`, etc.) and therefore can be included in the P&L report, `FALSE` otherwise.
 */
gboolean is_p_l_account(gchar *name, GtkTreeIter iter, Data_passer *data_passer) {
	/**

	* Array of account names that can be included in a P&L report. Need to make this dynamic.
	* \struct PL_ACCOUNTS_ARRAY
	*/
	const gchar *PL_ACCOUNTS_ARRAY[] = {"12201", "242", "323", "325", "349", "351", "353", "9820"};

	const int LENGTH_PL_ACCOUNTS_ARRAY = 8;

	GtkTreePath *tree_path = gtk_tree_model_get_path(GTK_TREE_MODEL(data_passer->accounts_store), &iter);
	gboolean is_pl_account = FALSE;

	for (gint i = 0; i < LENGTH_PL_ACCOUNTS_ARRAY; i++) {
		if (g_strcmp0(name, PL_ACCOUNTS_ARRAY[i]) == 0) {
			is_pl_account = TRUE;
			break;
		}
	}
	gtk_tree_path_free(tree_path);
	return is_pl_account;
}

/**
 * Gets the fixed asset associated with the name of a passed GnuCash account. For example, if the passed GnuCash account has name `323`, this function returns the iter in the reports tree pointing to `323 Main Street.`.
 * @param model Pointer to the report model.
 * @param account_iter Pointer to the GnuCash account.
 * @param corresponding_report_iter Pointer to the corresponding fixed asset in the report tree.
 * @param data_passer Pointer to a Data_passer struct.
 * @return `TRUE` if a matching fixed asset was found in the report tree, `FALSE` otherwise.
 */
gboolean get_report_tree_fixed_asset(GtkTreeModel *model, GtkTreeIter account_iter, GtkTreeIter *corresponding_report_iter, Data_passer *data_passer) {
	gboolean return_value = FALSE;
	gchar *name; /* Memory freed below */
	gtk_tree_model_get(model, &account_iter, NAME_ACCOUNT, &name, -1);

	GtkTreeIter report_tree_iter;
	GtkTreeModel *report_model = GTK_TREE_MODEL(data_passer->reports_store);

	gtk_tree_model_get_iter_first(report_model, &report_tree_iter);

	/*
	Iterate over the top level fixed assets in the reports tree.
	For each asset:
	* See if the account name is in the current fixed asset's name.
	* If it is then the current report iter is the ancestor of the selected account in the reports tree. Break out of the loop.
	*/
	do {
		gchar *report_account_description; /*Memory freed below */
		gtk_tree_model_get(report_model, &report_tree_iter, DESCRIPTION_REPORT, &report_account_description, -1);

		if (strstr(report_account_description, name) != NULL) {
			*corresponding_report_iter = report_tree_iter;
			return_value = TRUE;
			g_free(report_account_description);
			break;
		}
		g_free(report_account_description);
	} while (gtk_tree_model_iter_next(report_model, &report_tree_iter));
	g_free(name);
	return return_value;
}

/**
 * Determines if a passed GnuCash account is already in the reports tree.
 * @param accounts_model Pointer to the report model.
 * @param accounts_iter Pointer to the GnuCash account.
 * @param data_passer Pointer to a Data_passer struct.
 * @return `TRUE` the passed GnuCash account is already in the reports tree, `FALSE` otherwise.
 */
gboolean account_parent_in_report_tree(GtkTreeModel *accounts_model, GtkTreeIter accounts_iter, Data_passer *data_passer) {
	/* Check if under income or expense.
		Get name (323, 242, 349, etc.), which indicates the fixed asset
		 Check if fixed asset is in top level of reports tree
	 */
	GtkTreePath *accounts_path_selection = gtk_tree_model_get_path(accounts_model, &accounts_iter);
	gboolean is_income_account = gtk_tree_path_is_descendant(accounts_path_selection, data_passer->income_root);
	gboolean is_expense_account = gtk_tree_path_is_descendant(accounts_path_selection, data_passer->expenses_root);

	if (is_income_account || is_expense_account) {
		/* Get the name */
		gchar *name; /* Memory freed below */
		gtk_tree_model_get(accounts_model, &accounts_iter, NAME_ACCOUNT, &name, -1);

		/* See if the name is in one of the top-level report accounts. */
		GtkTreeIter report_tree_iter;
		GtkTreeModel *report_model = GTK_TREE_MODEL(data_passer->reports_store);

		gtk_tree_model_get_iter_first(report_model, &report_tree_iter);

		do {
			gchar *report_account_description;
			gtk_tree_model_get(report_model, &report_tree_iter, DESCRIPTION_REPORT, &report_account_description, -1);
			if (strstr(report_account_description, name) != NULL) {
				g_free(report_account_description);
				g_free(name);
				return TRUE;
			}
			g_free(report_account_description);
		} while (gtk_tree_model_iter_next(report_model, &report_tree_iter));
		g_free(name);
	}
	return FALSE;
}

/**
 * Gtk callback fired when the selection in the GnuCash accounts tree view changes. This callback sets the add button's sensitivity to `TRUE` if the selected account can be added to the report, and sets the sensitivity to `FALSE` if the selected account cannot be added.
 *
 * The following conditions must be met for enabling a selected account to be added to the reports tree:
 *
 * - The selected account does not yet exist in the reports tree, AND one of the following two conditions:
 *   - The selected account is a child of the Fixed Assets root (meaning it is one of the top-level property fixed assets), OR
 *   - The selected account is eligible to be in the P&L report.
 *
 * @param tree_view_accounts Pointer to the accounts tree view.
 * @param user_data Pointer to a Data_passer struct.
 * @see is_guid_in_reports_tree()
 * @see is_p_l_account()
 * @see account_parent_in_report_tree()
 */
void account_tree_cursor_changed(GtkTreeView *tree_view_accounts, gpointer user_data) {
	Data_passer *data_passer = (Data_passer *)user_data;
	GtkTreeSelection *tree_view_accounts_selection = gtk_tree_view_get_selection(tree_view_accounts);

	gtk_tree_selection_set_mode(tree_view_accounts_selection, GTK_SELECTION_SINGLE);
	GtkTreeModel *model = gtk_tree_view_get_model(tree_view_accounts);
	GtkTreeIter iter; /* Attempts to free this memory resulted in seg fault */
	gtk_tree_selection_get_selected(tree_view_accounts_selection, &model, &iter);
	gchar *guid; /* Memory freed in three places, need to make this easier. */
	gchar *name; /* Memory freed in three places, need to make this easier. */
	gtk_tree_model_get(model, &iter, GUID_ACCOUNT, &guid, NAME_ACCOUNT, &name, -1);

	/* Check if the currently selected account is already in the reports list. */
	data_passer->is_guid_in_reports_tree = FALSE;
	GtkTreeIter reports_root_iter;
	gtk_tree_model_get_iter_first(GTK_TREE_MODEL(data_passer->reports_store), &reports_root_iter);
	is_guid_in_reports_tree(data_passer->reports_store, reports_root_iter, guid, data_passer);

	if (data_passer->is_guid_in_reports_tree == FALSE) {
		GtkTreePath *currently_selected_path = gtk_tree_model_get_path(model, &iter);
		if (gtk_tree_path_is_descendant(currently_selected_path, data_passer->fixed_asset_root)) {
			gtk_widget_set_sensitive(data_passer->btn_add, TRUE);
			gtk_tree_path_free(currently_selected_path);
			/* We free the memory in three difference places in this function, need to find a way to free it once. */
			g_free(guid);
			g_free(name);
			return;
		}

		if (is_p_l_account(name, iter, data_passer) &&
			account_parent_in_report_tree(model, iter, data_passer)) {
			/* If the selected account is NOT already in the reports list, and if it
			   is an account that is eligible to be in the reports list, set
			   the add button's sensitivity. */
			gtk_widget_set_sensitive(data_passer->btn_add, TRUE);

			g_free(guid);
			g_free(name);
			return;
		}
	}
	/* If the selected account is already in the reports list, clear the add button's sensitivity. */
	gtk_widget_set_sensitive(data_passer->btn_add, FALSE);
	g_free(guid);
	g_free(name);
}

/**
 * Gtk callback fired when user clicks the add button. This function adds a currently selected account to the reports tree.
 *
 * The logic is as follows:

	-# If the selection is a fixed asset, add it to the reports tree and exit.
	-# Get to which fixed asset the selection belongs.
	-# Determine if the selection is an expense or income.
	-# Add the selection to the corresponding account in the reports tree, under expense or income as appropriate.
 * @param button Pointer to the clicked add button.
 * @param user_data Pointer to a Data_passer struct.
 */
void add_account_to_reports(GtkButton *button, gpointer user_data) {
	Data_passer *data_passer = (Data_passer *)user_data;

	GtkTreeSelection *tree_view_accounts_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data_passer->tree_view_accounts));

	/* Get iter of selected account. */
	gtk_tree_selection_set_mode(tree_view_accounts_selection, GTK_SELECTION_SINGLE);
	GtkTreeModel *accounts_model = gtk_tree_view_get_model(GTK_TREE_VIEW(data_passer->tree_view_accounts));
	GtkTreeIter iter_selection;
	gtk_tree_selection_get_selected(tree_view_accounts_selection, &accounts_model, &iter_selection);

	/* Get iter and path of selected account's parent. */
	GtkTreeIter iter_parent;
	gtk_tree_model_iter_parent(accounts_model, &iter_parent, &iter_selection);
	/* Memory freed below */
	GtkTreePath *parent_tree_path = gtk_tree_model_get_path(accounts_model, &iter_parent);

	/* If parent's path is same as the path of the fixed asset root, then user selected a fixed asset. Add it to reports model and exit. */
	if (gtk_tree_path_compare(data_passer->fixed_asset_root, parent_tree_path) == 0) {
		GtkTreeIter iter_child;
		gchar *guid; /* Memory freed below */
		gtk_tree_model_get(accounts_model, &iter_selection, GUID_ACCOUNT, &guid, -1);
		gchar *description; /* Memory freed below */
		gtk_tree_model_get(accounts_model, &iter_selection, DESCRIPTION_ACCOUNT, &description, -1);

		gtk_tree_store_append(data_passer->reports_store, &iter_child, NULL);
		gtk_tree_store_set(data_passer->reports_store, &iter_child, GUID_REPORT, guid, DESCRIPTION_REPORT, description, -1);

		GtkTreeIter child_iter_income;
		gtk_tree_store_append(data_passer->reports_store, &child_iter_income, &iter_child);
		gtk_tree_store_set(data_passer->reports_store, &child_iter_income, DESCRIPTION_REPORT, REVENUE, -1);

		GtkTreeIter child_iter_expenses;
		gtk_tree_store_append(data_passer->reports_store, &child_iter_expenses, &iter_child);
		gtk_tree_store_set(data_passer->reports_store, &child_iter_expenses, DESCRIPTION_REPORT, EXPENSES, -1);

		g_free(guid);
		g_free(description);
		gtk_tree_path_free(parent_tree_path);
		gtk_widget_set_sensitive(data_passer->btn_add, FALSE);
		return;
	}
	gtk_tree_path_free(parent_tree_path);

	/* Determine if selection is an expense or income account. */
	GtkTreePath *tree_path_selection = gtk_tree_model_get_path(accounts_model, &iter_selection);
	gboolean is_income_account = gtk_tree_path_is_descendant(tree_path_selection, data_passer->income_root);
	gtk_tree_path_free(tree_path_selection);

	/* Find the iter in the reports tree of the fixed asset. */
	GtkTreeIter corresponding_report_iter;
	if (get_report_tree_fixed_asset(accounts_model, iter_selection, &corresponding_report_iter, data_passer)) {
		/* Find the iter of the fixed asset's income or expense list */

		GtkTreeModel *reports_model = GTK_TREE_MODEL(data_passer->reports_store);
		GtkTreeIter income_expense_parent;
		gboolean found_income_expense_parent = gtk_tree_model_iter_children(reports_model, &income_expense_parent, &corresponding_report_iter);

		/*
			Iterate over the fixed asset's two child accounts Revenue and Expenses. One of those children is the parent where
			we will add the selected account.
			* Examine the current child.
			* If it is "Revenue," and if the selected account is a revenue account,
			  then this child is the parent where we add the selected account.
			* If it is "Expense," and if the selected account is an expense account,
			  then this child is the parent where we add the selected account.
		*/
		if (found_income_expense_parent) {
			do {
				gchar *revenue_or_expenses; /* Memory freed below */
				gtk_tree_model_get(reports_model, &income_expense_parent, DESCRIPTION_REPORT, &revenue_or_expenses, -1);

				if (((strstr(revenue_or_expenses, REVENUE) != NULL) && is_income_account) ||
					((strstr(revenue_or_expenses, EXPENSES) != NULL) && !is_income_account)) {
					g_free(revenue_or_expenses);
					break;
				}
				g_free(revenue_or_expenses);
			} while (gtk_tree_model_iter_next(reports_model, &income_expense_parent));

			gchar *guid; /* Memory freed below */
			gtk_tree_model_get(accounts_model, &iter_selection, GUID_ACCOUNT, &guid, -1);
			gchar *name; /* Memory freed below */
			gtk_tree_model_get(accounts_model, &iter_parent, NAME_ACCOUNT, &name, -1);

			GtkTreeIter new_income_expense_entry;
			gtk_tree_store_append(data_passer->reports_store, &new_income_expense_entry, &income_expense_parent);
			gtk_tree_store_set(data_passer->reports_store, &new_income_expense_entry, GUID_REPORT, guid, DESCRIPTION_REPORT, name, -1);
			g_free(guid);
			g_free(name);
			gtk_widget_set_sensitive(data_passer->btn_add, FALSE);
		}
	}
}
