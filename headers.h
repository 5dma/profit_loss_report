#include <glib-2.0/glib.h>
#include <gtk/gtk.h>
#include <sqlite3.h>
#include <stdio.h>
#ifndef __HEADER
#define __HEADER

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

typedef struct {
    sqlite3 *db;
    gchar *start_date;
    gchar *end_date;
    GSList *properties;
    FILE *output_file;
    gdouble total_revenues;
    gdouble total_expenses;
    gboolean subtotaling_revenues;
    GApplication *app;
    GtkTreeStore *accounts_store;
    GtkTreeStore *reports_store;
    GtkWidget *btn_add;
    GtkWidget *btn_delete;
} Data_passer;

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

void make_pl_report(gpointer user_data);
void generate_property_report(Property *property, Data_passer *data_passer);
void cleanup(Data_passer *data_passer);
void read_accounts_tree(Data_passer *data_passer);
void read_reports_tree(Data_passer *data_passer);

void account_tree_cursor_changed(GtkTreeView *tree_view_accounts, gpointer user_data);


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
