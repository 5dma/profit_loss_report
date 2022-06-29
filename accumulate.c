#include <stdio.h>

#include "headers.h"

static int total_up_income(void *user_data, int argc, char **argv, char **azColName) {
    Account_summary *account_summary = (Account_summary *)user_data;
    if (g_strcmp0(argv[0], "0") == 0) {
        account_summary->subtotal = 0;
    } else {
        gchar *end_ptr;
        account_summary->subtotal = g_ascii_strtod(argv[1], &end_ptr);
    }
    return 0;
}

void make_subtotals(gpointer data, gpointer user_data) {
    Account_summary *income_account = (Account_summary *)data;
    Data_passer *data_passer = (Data_passer *)user_data;

    gfloat total;

    int rc;
    char sql[1000];
    char *zErrMsg = 0;

    gint num_bytes = g_snprintf(sql, 1000, SUM_OF_ACCOUNT_ACTIVITY, income_account->guid, income_account->guid, data_passer->start_date);

    rc = sqlite3_exec(data_passer->db, sql, total_up_income, income_account, &zErrMsg);

    if (rc != SQLITE_OK) {
        g_print("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
  //      g_print("Table created successfully\n");
    }

    if (data_passer->subtotaling_revenues) {
        data_passer->total_revenues += income_account->subtotal;
    } else {
        data_passer->total_expenses += income_account->subtotal;
    }

    fprintf(data_passer->output_file, ACCOUNT_REPORT, income_account->description, income_account->subtotal);
}

void make_property_report(gpointer data, gpointer user_data) {
    Property *property = (Property *)data;
    Data_passer *data_passer = (Data_passer *)user_data;
    fprintf(data_passer->output_file, PROPERTY_HEADER, property->description);
    data_passer->total_revenues = 0;
    data_passer->total_expenses = 0;
    fputs(INCOME_HEADER, data_passer->output_file);

    data_passer->subtotaling_revenues = TRUE;
    g_slist_foreach(property->income_accounts, make_subtotals, data_passer);
    fprintf(data_passer->output_file, INCOME_TOTAL, data_passer->total_revenues);

    fputs(EXPENSE_HEADER, data_passer->output_file);
    data_passer->subtotaling_revenues = FALSE;
    g_slist_foreach(property->expense_accounts, make_subtotals, data_passer);

   fprintf(data_passer->output_file, EXPENSE_TOTAL, data_passer->total_expenses);

    fprintf(data_passer->output_file, NET_INCOME, data_passer->total_revenues - data_passer->total_expenses);

    fputs("</table>\n", data_passer->output_file);
}

void make_pl_report(gpointer user_data) {
    Data_passer *data_passer = (Data_passer *)user_data;

    gchar *report_start = "<!DOCTYPE HTML>\n<html lang=\"en\">\n<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n<title>P &amp; L Rental Properties</title>\n<link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.0.2/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-EVSTQN3/azprG1Anm3QDgpJLIm9Nao0Yz1ztcQTwFspd3yD65VohhpuuCOmLASjC\" crossorigin=\"anonymous\">\n<style>\ntd:nth-child(2) {text-align: right;}\n.single_underline {text-decoration: underline;}\n.double_underline {border-bottom:double black;}\n.left_indent {padding-left: 10px}\nh3 {margin-top: 50px;}\n</style>\n</head>\n<body class=\"p-4\">\n";

    gchar *report_end = "</body>\n</html>";

    fputs(report_start, data_passer->output_file);

    g_slist_foreach(data_passer->properties, make_property_report, data_passer);

    fputs(report_end, data_passer->output_file);
}