#include "headers.h"

void write_subtotals(gpointer data, gpointer user_data) {
    gdouble *total = (gdouble *)user_data;
    Account_summary *account_summary = (Account_summary *)data;
    g_print("%s:  %-#4.2f\n", account_summary->description,account_summary->subtotal);
    *total += account_summary->subtotal;
}

void generate_property_report(Property *property, Data_passer *data_passer) {

    gdouble total_revenues;
    gdouble total_expenses;
    g_print("%s\n",property->description);
    g_print("Revenues\n");
    g_slist_foreach(property->income_accounts,write_subtotals,&total_revenues);
    g_print("Expenses\n");
    g_slist_foreach(property->expense_accounts,write_subtotals, &total_expenses);
    g_print("Net income: %-#4.2f\n",total_revenues - total_expenses);

    gchar *property_heading = "<h3>9820 Georgia Ave., Silver Spring, MD</h3>\n<table class=\"table table-bordered\" style=\"width: 50%;\">\n<tr class=\"table-primary\">\n<td colspan=\"2\">Income</td></tr>";
    

}