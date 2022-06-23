#include <glib-2.0/glib.h>
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
} Data_passer;

Data_passer *setup ();

void make_pl_report(gpointer user_data);
void generate_property_report(Property *property, Data_passer *data_passer);
void cleanup(Data_passer *data_passer);

enum account_type{INCOME, EXPENSE };

#endif
