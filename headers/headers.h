#include <glib-2.0/glib.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include <sqlite3.h>
#include <stdio.h>

/**
 * @file headers.h
 * @brief Contains data structures and function prototypes.
 */

#define GUID_LENGTH 40
/**
 * Structure for passing an iterator to functions and callbacks.
 * \struct Settings_passer
 */
typedef struct {
	GtkWidget *settings_window; /**< Pointer to the settings window. */
	GtkWidget *text_output_filename; /**< Pointer to entry field holding the output file name. */
	GtkWidget *text_sqlite_filename; /**< Pointer to entry field holding the path to the SQLite file. */
	GtkWidget *start_calendar; /**< Pointer to the calendar for the start date. */
	GtkWidget *end_calendar; /**< Pointer to the calendar for the end date. */
	gint current_year; /**< Holds the current year. */
	gint current_month; /**< Holds the current month. */
	gint current_day; /**< Holds the current day. */
	gboolean using_today_date; /**< Indicates user is using today's date. Required to manage appearance of end date calendar and checkbox. */
} Settings_passer;

/**
 * Structure for passing data to functions and callbacks.
 * \struct Data_passer
 */
typedef struct {
	gchar *sqlite_path; /**< Path to sqlite database. */
	sqlite3 *db; /**< Pointer to sqlite database handle. */
	gchar *start_date; /**< Start of date range for reporting purposes. */
	gchar *end_date; /**< End of date range for reporting purposes. */
	FILE *output_file; /**< Handle for output file. */
	gchar *output_file_name; /**< Path to output file. */
	gdouble total_revenues; /**< Accumulates total revenue for a property. */
	gdouble total_expenses; /**< Accumulates total expenses for a property. */
	gboolean subtotaling_revenues; /**< Indicates if we are subtotaling revenues or expenses for a property. */
	GApplication *app; /**< Pointer to the GTK application. */
	GtkTreeStore *accounts_store; /**< Pointer to tree store for GnuCash accounts. */
	GtkTreeStore *reports_store; /**< Pointer to tree store for accounts included in the P&L report. */
	GtkWidget *tree_view_accounts; /**< Pointer to tree view showing GnuCash accounts. */
	GtkWidget *tree_view_reports; /**< Pointer to tree view showing accounts included in the P&L report. */
	GtkWidget *btn_add; /**< Pointer to the add button. */
	GtkWidget *btn_delete; /**< Pointer to the delete button. */
	GtkTreePath *fixed_asset_root; /**< Path in the GnuCash tree that holds the parent of all fixed assets. */
	GtkTreePath *income_root; /**< Path in the GnuCash tree that holds the parent of all income accounts. */
	GtkTreePath *expenses_root; /**< Path in the GnuCash tree that holds the parent of all expense accounts. */
	gboolean is_guid_in_reports_tree; /**< Indicates if a selected guid is already in the reports tree. */
	GList *iters_to_be_freed; /**< List of iters that need to be freed after they are created in read_accounts_tree() and build_tree(). */
	GtkWidget *window; /**< Pointer to the main application window. */
	gulong handler; /**< ID of the handler associated with the cursor-changed event in the tree_vew_accounts. */
	GtkWidget *status_bar; /**< Pointer to the status bar. */
	guint status_bar_context; /**< ID of the status bar's context. There is only one status-bar context in this application. */
	guint error_condition; /**< Indication of an error condition. See error_condition. */
	JsonObject *root_obj; /**< Pointer to the root JSON object in the file accounts.json. */
	Settings_passer *settings_passer; /**< Pointer to a struct Settings_passer. */
	GDateTime *current_date_time; /**< Pointer to current date and time. Calendars are set to this date if no start date or end date appear in the configuration file. In future, will be used to print run date on report. */
} Data_passer;

/**
 * Structure for passing an iterator to functions and callbacks.
 * \struct Iter_passer
 */
typedef struct {
	sqlite3 *db; /**< Pointer to sqlite database handle. */
	GtkTreeStore *accounts_store; /**< Pointer to GnuCash accounts store. */
	int number_of_children; /**< Number of children associated with `parent`. */
	GtkTreeIter *parent; /**< Iter to a given parent account in the GnuCash accounts store. */
	GtkTreeIter *child; /**< Iter to a child account of `parent`. */
	gboolean at_root_level; /**< `TRUE` if the parent is at the root level of the GnuCash store, `FALSE` otherwise. */
	GList *iters_to_be_freed; /**< List of iters that need to be freed after they are created in read_accounts_tree() and build_tree(). */
	Data_passer *data_passer; /**< Pointer to the Data_passer */
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

void make_settings_dialog(GtkButton *button, gpointer user_data);

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

/**
 * Descriptors of an error condition's severity level.
 * \enum error_condition
 */
enum error_condition {
	NO_DATABASE_CONNECTION, /**< Could not connect to the sqlite database. */
	SQLITE_SELECT_FAILURE, /**< Could not perform an sqlite SELECT. */
	JSON_PROCESSING_FAILURE, /**< Could not perform an operation on a JSON object. */
	REPORT_GENERATION_FAILURE, /**< Could not generate the HTML report. */
	NONE /**< No error condition. */
};

void on_app_activate(GApplication *app, gpointer data);
GtkWidget *make_window(Data_passer *data_passer);

#define REVENUE "Revenue"
#define EXPENSES "Expenses"
// static gchar *REVENUE = "Revenue";   /**< String constant for adding a `Revenue` heading in the P&L report's store. */
// static gchar *EXPENSES = "Expenses"; /**< String constant for adding an `Expenses` heading in the P&L report's store. */

#define SELECT_DESCRIPTION_FROM_ACCOUNT "SELECT description FROM accounts WHERE guid = \"%s\";" /**< SQL statement that retrieves a description for a given guid. See get_account_description().  */

#define SELECT_DESCRIPTION_FROM_PARENT_ACCOUNT "SELECT parent.name FROM accounts child JOIN accounts parent ON child.parent_guid = parent.guid WHERE child.guid=\"%s\";" /**< SQL statement that, for a given guid, retrieves the parent guid's description. See get_parent_account_description(). */

#define SUM_OF_ACCOUNT_ACTIVITY "SELECT COUNT(*), ABS(SUM(CAST(value_num AS REAL)/value_denom)), (SELECT parent.description FROM accounts child JOIN accounts parent ON child.parent_guid = parent.guid WHERE child.guid=\"%s\") FROM splits LEFT JOIN transactions ON tx_guid = transactions.guid WHERE account_guid = \"%s\" AND post_date >= \"%s 00:00:00\" AND post_date <= \"%s 23:59:59\";" /**< SQL statement that, for a given guid, retrieves the number of transactions and the subtotal of those transactions. See make_subtotals(). */

/* String templates for HTML output */
#define DATE_RANGE "<p class=\"text-center\">For the period %s to %s</p>\n" /**< String template for displaying a date range. See make_pl_report().  */

#define ACCOUNT_REPORT "<tr>\n<td><span class=\"left_indent\">%s</span></td>\n<td>%s</td>\n</tr>\n" /**< HTML template for printing an account's subtotal. See make_subtotals(). */
#define PROPERTY_HEADER "<h3>%s</h3>\n<table class=\"table table-bordered\" style=\"width: 50%%;\">\n" /**< HTML template for printing a fixed asset's header. See make_property_report(). */
#define INCOME_HEADER "<tr class=\"table-primary\">\n<td colspan=\"2\">Income</td></tr>\n" /**< HTML template for printing the income header in a fixed asset's report. See make_property_report(). */
#define INCOME_TOTAL "<tr>\n<td>Total income</td>\n<td class=\"single_underline\">%s</td>\n</tr>\n" /**< HTML template for printing the total income in a fixed asset's report. See make_property_report(). */
#define EXPENSE_HEADER "<tr class=\"table-primary\">\n<td colspan=\"2\">Expenses</td></tr>\n" /**< HTML template for printing the expense header in a fixed asset's report. See make_property_report(). */
#define EXPENSE_TOTAL "<tr>\n<td>Total expenses</td>\n<td class=\"single_underline\">%s</td>\n</tr>\n" /**< HTML template for printing the total expenses in a fixed asset's report. See make_property_report(). */
#define NET_INCOME "<tr class=\"table-success\">\n<td>Net income</td>\n<td><span class=\"double_underline\">%s</span></td>\n</tr>\n" /**< HTML template for printing the net income (INCOME_TOTAL − EXPENSE TOTAL) in a fixed asset's report. See make_property_report(). */

#define START_DATE_SUFFIX " 00:00:00" /**< Suffix appended to the date of a selected start date. See save_date(). */
#define END_DATE_SUFFIX " 23:59:59" /**< Suffix appended to the date of a selected end date. See save_date(). */
