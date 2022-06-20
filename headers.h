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
    GSList *income_transactions;
    GSList *expense_transactions;
} Property;

typedef struct {
    gchar *split_guid;
    gchar *transaction_id;
    gdouble value;
    GDateTime *post_date;
} Transaction;

typedef struct {
    sqlite3 *db;
    GDateTime *start_date;
    GDateTime *end_date;
     GTimeZone *default_tz;
    GSList *properties;
} Data_passer;

Data_passer *setup ();
void accumulate_income(gpointer data, gpointer user_data);

#endif
