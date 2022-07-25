#include <glib-2.0/glib.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include <sqlite3.h>
#include <stdio.h>
#ifndef __HEADER
#define __HEADER

/**
 * @file headers.h
 * @brief Contains data structures and function prototypes.
 */

/**
 * Structure for passing an iter to functions and callbacks.
 * \struct Settings_passer
 */
typedef struct {
    GtkWidget *settings_window;      /**< Pointer to the settings window. */
    GtkWidget *text_output_filename; /**< Pointer to entry field holding the output file name. */
    GtkWidget *text_sqlite_filename; /**< Pointer to entry field holding the path to the SQLite file. */
    GtkWidget *start_calendar;       /**< Pointer to the calendar for the start date. */
    GtkWidget *end_calendar;         /**< Pointer to the calendar for the end date. */
} Settings_passer;

/**
 * Structure for passing data to functions and callbacks.
 * \struct Data_passer
 */
typedef struct {
    gchar *sqlite_path;               /**< Path to sqlite database. */
    sqlite3 *db;                      /**< Pointer to sqlite database handle. */
    gchar *start_date;                /**< Start of date range for reporting purposes. */
    gchar *end_date;                  /**< End of date range for reporting purposes. */
    FILE *output_file;                /**< Handle for output file. */
    gchar *output_file_name;          /**< Path to output file. */
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
    GList *iters_to_be_freed;         /**< List of iters that need to be freed after they are created in read_accounts_tree() and build_tree(). */
    GtkWidget *window;                /**< Pointer to the main application window. */
    gulong handler;                   /**< ID of the handler associated with the cursor-changed event in the tree_vew_accounts. */
    GtkWidget *status_bar;            /**< Pointer to the status bar. */
    guint status_bar_context;         /**< ID of the status bar's context. There is only one status-bar context in this application. */
    guint error_condition;            /**< Indication of an error condition. See error_condition. */
    JsonObject *root_obj;             /**< Pointer to the root JSON object in the file accounts.json. */
    Settings_passer *settings_passer; /**< Pointer to a struct Settings_passer. */
    GDateTime *current_date_time;     /**< Pointer to current date and time. */
} Data_passer;

/**
 * Structure for passing an iter to functions and callbacks.
 * \struct Iter_passer
 */
typedef struct {
    sqlite3 *db;                  /**< Pointer to sqlite database handle. */
    GtkTreeStore *accounts_store; /**< Pointer to GnuCash accounts store. */
    int number_of_children;       /**< Number of children associated with `parent`. */
    GtkTreeIter *parent;          /**< Iter to a given parent account in the GnuCash accounts store. */
    GtkTreeIter *child;           /**< Iter to a child account of `parent`. */
    gboolean at_root_level;       /**< `TRUE` if the parent is at the root level of the GnuCash store, `FALSE` otherwise. */
    GList *iters_to_be_freed;     /**< List of iters that need to be freed after they are created in read_accounts_tree() and build_tree(). */
    Data_passer *data_passer;     /**< Pointer to the Data_passer */
} Iter_passer;

Data_passer *setup();

void make_pl_report(GtkButton *button, gpointer user_data);
void cleanup(GtkWidget *window, gpointer user_data);
void read_accounts_tree(Data_passer *data_passer);
void add_account_to_reports(GtkButton *button, gpointer user_data);
void account_tree_cursor_changed(GtkTreeView *tree_view_accounts, gpointer user_data);

void add_accounts(Data_passer *data_passer, JsonObject *property_object, GtkTreeIter *property_iter, gint account_type);

void delete_account_from_reports(GtkButton *button, gpointer user_data);
void reports_tree_cursor_changed(GtkTreeView *tree_view_accounts, gpointer user_data);
void is_guid_in_reports_tree(GtkTreeStore *reports_store, GtkTreeIter current_iter, char *guid, Data_passer *data_passer);
void read_properties_into_reports_store(Data_passer *data_passer);

void get_account_description(const gchar *guid, gchar *description, Data_passer *data_passer);
void get_parent_account_description(gchar *guid, gchar *description, gpointer user_data);

void revert_report_tree(GtkButton *button, gpointer user_data);
void save_report_tree(GtkButton *button, gpointer user_data);

void show_settings(GtkButton *button, gpointer user_data);

GtkWidget *make_settings_dialog(Data_passer *data_passer);

/* Prototypes for the settings dialog */
void close_settings(GtkButton *button, gpointer user_data);
void get_output_filename(GtkButton *button, gpointer user_data);
void get_sqlite_filename(GtkButton *button, gpointer user_data);
void save_date(GtkCalendar *calendar, gpointer user_data);

void closeup(GtkWidget *button_close, gpointer data);
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
    GUID_ACCOUNT,        /**< Column 0 holds the the GnuCash guid. */
    NAME_ACCOUNT,        /**< Column 1 holds the the GnuCash account's name. */
    DESCRIPTION_ACCOUNT, /**< Column 2 holds the the GnuCash account's description. */
    COLUMNS_ACCOUNT      /**< Number of columns in the GnuCash accounts store. */
};

/**
 * Counter for columns in the P&L report's store.
 */
enum report_store_fields {
    GUID_REPORT,        /**< Column 0 holds the the GnuCash guid. */
    DESCRIPTION_REPORT, /**< Column 1 holds the the GnuCash account's description. */
    COLUMNS_REPORT      /**< Number of columns in the P&L report's store. */
};

/**
 * \enum error_condition
 * Descriptors of an error condition's severity level.
 */
enum error_condition {
    NO_DATABASE_CONNECTION,    /**< Could not connect to the sqlite database. */
    SQLITE_SELECT_FAILURE,     /**< Could not perform an sqlite SELECT. */
    JSON_PROCESSING_FAILURE,   /**< Could not perform an operation on a JSON object. */
    REPORT_GENERATION_FAILURE, /**< Could not generate the HTML report. */
    NONE                       /**< No error condition. */
};

static const gint LENGTH_PL_ACCOUNTS_ARRAY = 8; /**< Need to get rid of this, along with PL_ACCOUNTS_ARRAY. */

/**
 * \struct PL_ACCOUNTS_ARRAY
 * Array of account names that can be included in a P&L report. NEED TO MAKE THIS DYNAMIC.
 */
static const gchar *PL_ACCOUNTS_ARRAY[] = {"12201", "242", "323", "325", "349", "351", "353", "9820"};
typedef enum {
    STRING,
} target_info;

void on_app_activate(GApplication *app, gpointer data);
GtkWidget *make_window(Data_passer *data_passer);

static gchar *REVENUE = "Revenue";   /**< String constant for adding a `Revenue` heading in the P&L report's store. */
static gchar *EXPENSES = "Expenses"; /**< String constant for adding an `Expenses` heading in the P&L report's store. */

static gchar *SELECT_DESCRIPTION_FROM_ACCOUNT = "SELECT description FROM accounts WHERE guid = \"%s\";"; /**< SQL statement that retrieves a description for a given guid. See get_account_description().  */

static gchar *SELECT_DESCRIPTION_FROM_PARENT_ACCOUNT = "SELECT parent.description FROM accounts child JOIN accounts parent ON child.parent_guid = parent.guid WHERE child.guid=\"%s\";"; /**< SQL statement that, for a given guid, retrieves the parent guid's description. See get_parent_account_description(). */

static gchar *SUM_OF_ACCOUNT_ACTIVITY = "SELECT COUNT(*), ABS(SUM(value_num/value_denom)), (SELECT parent.description FROM accounts child JOIN accounts parent ON child.parent_guid = parent.guid WHERE child.guid=\"%s\") FROM splits LEFT JOIN transactions ON tx_guid = transactions.guid WHERE account_guid = \"%s\" AND post_date > \"%s\";"; /**< SQL statement that, for a given guid, retrieves the number of transactions and the subtotal of those transactions. See make_subtotals(). */

/* String templates for HTML output */

static gchar *DATE_RANGE = "<p class=\"text-center\">For the period %s to %s</p>\n";
static gchar *RUN_DATE = "<p class=\"text-center\">Run date: %s</p>\n";


static gchar *ACCOUNT_REPORT = "<tr>\n<td><span class=\"left_indent\">%s</span></td>\n<td>%-#4.2f</td>\n</tr>\n";                                  /**< HTML template for printing an account's subtotal. See make_subtotals(). */
static gchar *PROPERTY_HEADER = "<h3>%s</h3>\n<table class=\"table table-bordered\" style=\"width: 50%;\">\n";                                     /**< HTML template for printing a fixed asset's header. See make_property_report(). */
static gchar *INCOME_HEADER = "<tr class=\"table-primary\">\n<td colspan=\"2\">Income</td></tr>\n";                                                /**< HTML template for printing the income header in a fixed asset's report. See make_property_report(). */
static gchar *INCOME_TOTAL = "<tr>\n<td>Total income</td>\n<td class=\"single_underline\">%-#4.2f</td>\n</tr>\n";                                  /**< HTML template for printing the total income in a fixed asset's report. See make_property_report(). */
static gchar *EXPENSE_HEADER = "<tr class=\"table-primary\">\n<td colspan=\"2\">Expenses</td></tr>\n";                                             /**< HTML template for printing the expense header in a fixed asset's report. See make_property_report(). */
static gchar *EXPENSE_TOTAL = "<tr>\n<td>Total expenses</td>\n<td class=\"single_underline\">%-#4.2f</td>\n</tr>\n";                               /**< HTML template for printing the total expenses in a fixed asset's report. See make_property_report(). */
static gchar *NET_INCOME = "<tr class=\"table-success\">\n<td>Net income</td>\n<td><span class=\"double_underline\">%-#4.2f</span></td>\n</tr>\n"; /**< HTML template for printing the net income (INCOME_TOTAL − EXPENSE TOTAL) in a fixed asset's report. See make_property_report(). */

static gchar *START_DATE_SUFFIX = " 00:00:00"; /**< Suffix appended to the date of a selected start date. See save_date(). */
static gchar *END_DATE_SUFFIX = " 23:59:59"; /**< Suffix appended to the date of a selected end date. See save_date(). */


#endif
