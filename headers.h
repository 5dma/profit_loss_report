#include <glib-2.0/glib.h>
#include <sqlite3.h>
#ifndef __HEADER
#define __HEADER

typedef struct { 
    gchar *description;
    gdouble *amount;
} Line_Item;    

typedef struct {
    gchar *guid;
    gchar *description;
    GSList *income_accounts;
    GSList *expense_accounts;
    gchar *p_l_report_html;
} Property;

typedef struct {
    gchar *guid;
    gchar *description;
    gdouble subtotal;
} Account_summary;



typedef struct {
    gchar *split_guid;
    gchar *transaction_id;
    gdouble value;
    GDateTime *post_date;
} Transaction;

typedef struct {
    sqlite3 *db;
    gchar *start_date;
    gchar *end_date;
     GTimeZone *default_tz;
    GSList *properties;
} Data_passer;

Data_passer *setup ();
void accumulate_income(gpointer data, gpointer user_data);
void make_pl_report(gpointer data, gpointer user_data);
void generate_property_report(Property *property);

enum account_type{INCOME, EXPENSE };

#endif
