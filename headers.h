#include <glib-2.0/glib.h>
#include <json-glib/json-glib.h>
#include <gtk/gtk.h>
#include <sqlite3.h>
#include <stdio.h>
#ifndef __HEADER
#define __HEADER

/**
 * @file headers.h
 * @brief Contains data structures and function prototypes.
 */

/**
 * Structure for passing data to functions and callbacks.
*/ 
typedef struct {
    sqlite3 *db;                      /**< Pointer to sqlite database handle. */
    gchar *start_date;                /**< Start of date range for reporting purposes. */
    gchar *end_date;                  /**< End of date range for reporting purposes. */
    FILE *output_file;                /**< Handle for output file. */
    gdouble total_revenues;           /**< Accumulates total revenue for a property. */
    gdouble total_expenses;           /**< Accumulates total expenses for a property. */
    gboolean subtotaling_revenues;    /**< Indicates if we are subtotaling revenues or expenses for a property. */
    GApplication *app;                /**< Pointer to the GTK application. */
    GtkTreeStore *accounts_store;     /**< Pointer to tree store for GnuCash accounts. */
    GtkTreeStore *reports_store;      /**< Pointer to tree store for accounts included in the P&L report. */
    GtkWidget *tree_view_accounts;    /**< Pointer to tree view showing GnuCash accounts. */
    GtkWidget *tree_view_reports;     /**< Pointer to tree view showing accounts included in the P&L report. */
    GtkWidget *btn_add;               /**< Pointer to the add button. */
    GtkWidget *btn_delete;            /**< Pointer to the delete button. */
    GtkTreePath *fixed_asset_root;    /**< Path in the GnuCash tree that holds the parent of all fixed assets. */
    GtkTreePath *income_root;         /**< Path in the GnuCash tree that holds the parent of all income accounts. */
    GtkTreePath *expenses_root;       /**< Path in the GnuCash tree that holds the parent of all expense accounts. */
    gboolean is_guid_in_reports_tree; /**< Indicates if a selected guid is already in the reports tree. */
} Data_passer;

/**
 * Structure for passing an iter to functions and callbacks.
 * \struct Iter_passer
 */
typedef struct {
    sqlite3 *db;                  /**< Pointer to sqlite database handle. */
    GtkTreeStore *accounts_store; /**< Pointer to GnuCash accounts store. */
    int number_of_children;       /**< Number of children associated with `parent`. */
    GtkTreeIter parent;           /**< Iter to a given parent account in the GnuCash accounts store. */
    GtkTreeIter child;            /**< Iter to a child account of `parent`. */
    gboolean at_root_level;       /**< `TRUE` if the parent is at the root level of the GnuCash store, `FALSE` otherwise. */
} Iter_passer;

Data_passer *setup();

void make_pl_report(GtkButton *button, gpointer user_data);
void cleanup(GtkButton *btn_exit, Data_passer *data_passer);
void read_accounts_tree(Data_passer *data_passer);
void add_account_to_reports(GtkButton *button, gpointer user_data);
void account_tree_cursor_changed(GtkTreeView *tree_view_accounts, gpointer user_data);

void add_accounts(Data_passer *data_passer, JsonObject *property_object, GtkTreeIter *property_iter, gint account_type);

void delete_account_from_reports(GtkButton *button, gpointer user_data);
void reports_tree_cursor_changed(GtkTreeView *tree_view_accounts, gpointer user_data);
void is_guid_in_reports_tree(GtkTreeStore *reports_store, GtkTreeIter current_iter, char *guid, Data_passer *data_passer);
void read_properties_into_reports_store(Data_passer *data_passer);

void get_account_description(gchar *guid, gchar *description, gpointer user_data);
void get_parent_account_description(gchar *guid, gchar *description, gpointer user_data);

void revert_report_tree(GtkButton *button, gpointer user_data);
void save_report_tree(GtkButton *button, gpointer user_data);

/**
 * Flag used when computing subtotals for income or expenses.
 * @see make_subtotals()
 */
enum account_type { INCOME, /**< Indicates subtotals are for an income account. */
                    EXPENSE /**< Indicates subtotals are for an expense account. */
};

/**
 * Counter for columns in the GnuCash accounts store.
 */
enum account_store_fields {
    GUID_ACCOUNT, /**< Column 0 holds the the GnuCash guid. */
    NAME_ACCOUNT, /**< Column 1 holds the the GnuCash account's name. */
    DESCRIPTION_ACCOUNT, /**< Column 2 holds the the GnuCash account's description. */
    COLUMNS_ACCOUNT /**< Number of columns in the GnuCash accounts store. */
};

/**
 * Counter for columns in the P&L report's store.
 */
enum report_store_fields {
    GUID_REPORT, /**< Column 0 holds the the GnuCash guid. */
    DESCRIPTION_REPORT, /**< Column 1 holds the the GnuCash account's description. */
    COLUMNS_REPORT /**< Number of columns in the P&L report's store. */
};


static const gint LENGTH_PL_ACCOUNTS_ARRAY = 8; /**< Need to get rid of this, along with PL_ACCOUNTS_ARRAY. */

/**
 * \struct PL_ACCOUNTS_ARRAY
* Array of account names that can be included in a P&L report. NEED TO MAKE THIS DYNAMIC.
*/
static const gchar *PL_ACCOUNTS_ARRAY[] = {"12201", "242", "323", "325","349","351","353","9820"};
typedef enum {
    STRING,
} target_info;

void on_app_activate(GApplication *app, gpointer data);
GtkWidget *make_window(Data_passer *data_passer);

static gchar *REVENUE = "Revenue";
static gchar *EXPENSES = "Expenses";

static gchar *SELECT_DESCRIPTION_FROM_ACCOUNT = "SELECT description FROM accounts WHERE guid = \"%s\";";
static gchar *SELECT_DESCRIPTION_FROM_PARENT_ACCOUNT = "SELECT parent.description FROM accounts child JOIN accounts parent ON child.parent_guid = parent.guid WHERE child.guid=\"%s\";";
static gchar *SUM_OF_ACCOUNT_ACTIVITY = "SELECT COUNT(*), ABS(SUM(value_num/value_denom)), (SELECT parent.description FROM accounts child JOIN accounts parent ON child.parent_guid = parent.guid WHERE child.guid=\"%s\") FROM splits LEFT JOIN transactions ON tx_guid = transactions.guid WHERE account_guid = \"%s\" AND post_date > \"%s\";";
static gchar *ACCOUNT_REPORT = "<tr>\n<td><span class=\"left_indent\">%s</span></td>\n<td>%-#4.2f</td>\n</tr>\n";
static gchar *PROPERTY_HEADER = "<h3>%s</h3>\n<table class=\"table table-bordered\" style=\"width: 50%;\">\n";
static gchar *INCOME_HEADER = "<tr class=\"table-primary\">\n<td colspan=\"2\">Income</td></tr>\n";
static gchar *INCOME_TOTAL = "<tr>\n<td>Total income</td>\n<td class=\"single_underline\">%-#4.2f</td>\n</tr>\n";
static gchar *EXPENSE_HEADER = "<tr class=\"table-primary\">\n<td colspan=\"2\">Expenses</td></tr>\n";
static gchar *EXPENSE_TOTAL = "<tr>\n<td>Total expenses</td>\n<td class=\"single_underline\">%-#4.2f</td>\n</tr>\n";
static gchar *NET_INCOME = "<tr class=\"table-success\">\n<td>Net income</td>\n<td><span class=\"double_underline\">%-#4.2f</span></td>\n</tr>\n";
#endif
