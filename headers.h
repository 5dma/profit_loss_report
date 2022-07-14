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
typedef struct {
    gchar *guid;
    gchar *description;
    GSList *income_accounts;
    GSList *expense_accounts;
} Property;

typedef struct {
    gchar *guid;
    gchar *description;
    gdouble subtotal;
} Account_summary;

/**
 * \struct Data_passer
 * Structure for passing data to functions and callbacks.
*/ 
typedef struct {
    sqlite3 *db;
    gchar *start_date;
    gchar *end_date;
    FILE *output_file;
    gdouble total_revenues;
    gdouble total_expenses;
    gboolean subtotaling_revenues;
    GApplication *app;
    GtkTreeStore *accounts_store;
    GtkTreeStore *reports_store;
    /* Do we need the following two members? */
    GtkWidget *tree_view_accounts;
    GtkWidget *tree_view_reports;
    GtkWidget *btn_add;
    GtkWidget *btn_delete;
    GtkTreePath *fixed_asset_root;
    GtkTreePath *income_root;
    GtkTreePath *expenses_root;
    gboolean is_guid_in_reports_tree;
} Data_passer;

/**
 * \struct Iter_passer
 * Structure for passing an iter to functions and callbacks.
 * 
 * @see build_tree()
 * @see read_accounts_tree()
 * @see has_children()
*/ 
typedef struct {
    sqlite3 *db;
    GtkTreeStore *accounts_store;
    int number_of_children;
    GtkTreeIter parent;
    GtkTreeIter child;
    gboolean at_root_level;
} Iter_passer;

typedef struct {
    GtkTreeStore *reports_store;
    GtkTreeIter parent;
} Iter_passer_reports;

Data_passer *setup();

void make_pl_report(GtkButton *button, gpointer user_data);
void generate_property_report(Property *property, Data_passer *data_passer);
void cleanup(Data_passer *data_passer);
void read_accounts_tree(Data_passer *data_passer);
void read_reports_tree(Data_passer *data_passer);
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

enum account_type { INCOME,
                    EXPENSE };

enum account_store_fields {
    GUID_ACCOUNT,
    NAME_ACCOUNT,
    DESCRIPTION_ACCOUNT,
    COLUMNS_ACCOUNT
};

enum report_store_fields {
    GUID_REPORT,
    DESCRIPTION_REPORT,
    COLUMNS_REPORT
};

static const gint LENGTH_PL_ACCOUNTS_ARRAY = 8;
static const gchar *PL_ACCOUNTS_ARRAY[] = {"12201", "242", "323", "325","349","351","353","9820"};

typedef struct {
    gchar *guid;
    gchar *name;
    gchar *description;
} Account;

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
