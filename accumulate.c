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

    account_summary->description = g_strdup(argv[2]);
    return 0;
}

void make_subtotals(gpointer data, gpointer user_data) {
    Account_summary *income_account = (Account_summary *)data;
    Data_passer *data_passer = (Data_passer *)user_data;

    gfloat total;

    int rc;
    char sql[1000];
    char *zErrMsg = 0;
    /*   sql =
          "SELECT COUNT(*), ABS(SUM(value_num/value_denom)), (SELECT parent.description FROM accounts child JOIN accounts parent ON child.parent_guid = parent.guid WHERE child.guid="4a99ed7935764f35822f93f3526db596") FROM splits LEFT JOIN transactions ON tx_guid = transactions.guid WHERE account_guid = "4a99ed7935764f35822f93f3526db596" AND post_date > "2022-01-01 00:00:00";"; */

    gchar *format_string = "SELECT COUNT(*), ABS(SUM(value_num/value_denom)), (SELECT parent.description FROM accounts child JOIN accounts parent ON child.parent_guid = parent.guid WHERE child.guid=\"%s\") FROM splits LEFT JOIN transactions ON tx_guid = transactions.guid WHERE account_guid = \"%s\" AND post_date > \"%s\";";

    gint num_bytes = g_snprintf(sql, 1000, format_string, income_account->guid, income_account->guid, data_passer->start_date);

    rc = sqlite3_exec(data_passer->db, sql, total_up_income, income_account, &zErrMsg);

    if (rc != SQLITE_OK) {
        g_print("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        g_print("Table created successfully\n");
    }

    if (data_passer->subtotaling_revenues) {
    data_passer->total_revenues += income_account->subtotal;
    } else {
        data_passer->total_expenses += income_account->subtotal;
    }

    gchar *line_item = "<tr>\n<td><span class=\"left_indent\">%s</span></td>\n<td>%-#4.2f</td>\n</tr>\n";

    fprintf(data_passer->output_file, line_item, income_account->description, income_account->subtotal);
}

void make_property_report(gpointer data, gpointer user_data) {
    Property *property = (Property *)data;
    Data_passer *data_passer = (Data_passer *)user_data;
    gchar *property_header = "<h3>%s</h3>\n<table class=\"table table-bordered\" style=\"width: 50%;\">\n";
    fprintf(data_passer->output_file, property_header, property->description);
    data_passer->total_revenues = 0;
    data_passer->total_expenses = 0;
    gchar *income_header = "<tr class=\"table-primary\">\n<td colspan=\"2\">Income</td></tr>\n";
    fputs(income_header, data_passer->output_file);

    data_passer->subtotaling_revenues = TRUE;
    g_slist_foreach(property->income_accounts, make_subtotals, data_passer);
    gchar *income_total = "<tr>\n<td>Total income</td>\n<td class=\"single_underline\">%-#4.2f</td>\n</tr>\n";
    fprintf(data_passer->output_file, income_total,data_passer->total_revenues);

    gchar *expense_header = "<tr class=\"table-primary\">\n<td colspan=\"2\">Expenses</td></tr>\n";
    fputs(expense_header, data_passer->output_file);
    data_passer->subtotaling_revenues = FALSE;
    g_slist_foreach(property->expense_accounts, make_subtotals, data_passer);

 gchar *expenses_total = "<tr>\n<td>Total expenses</td>\n<td class=\"single_underline\">%-#4.2f</td>\n</tr>\n";
    fprintf(data_passer->output_file, expenses_total,data_passer->total_expenses);

    gchar *net_income = "<tr class=\"table-success\">\n<td>Net income</td>\n<td><span class=\"double_underline\">%-#4.2f</span></td>\n</tr>\n";

    fprintf(data_passer->output_file, net_income,data_passer->total_revenues - data_passer->total_expenses);


    fputs("</table>\n", data_passer->output_file);
}

void make_pl_report(gpointer user_data) {
    Data_passer *data_passer = (Data_passer *)user_data;

    gchar *report_start = "<!DOCTYPE HTML>\n<html lang=\"en\">\n<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n<title>P &amp; L Rental Properties</title>\n<link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.0.2/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-EVSTQN3/azprG1Anm3QDgpJLIm9Nao0Yz1ztcQTwFspd3yD65VohhpuuCOmLASjC\" crossorigin=\"anonymous\">\n<style>\ntd:nth-child(2) {text-align: right;}\n.single_underline {text-decoration: underline;}\n.double_underline {border-bottom:double black;}\n.left_indent {padding-left: 10px}\nh3 {margin-top: 50px;}\n</style>\n</head>\n<body class=\"p-4\">\n";

    gchar *report_end = "</body>\n</html>";

    fputs(report_start, data_passer->output_file);

    g_slist_foreach(data_passer->properties, make_property_report, data_passer);

    fputs(report_end, data_passer->output_file);

    /*    g_slist_foreach(property->income_accounts, make_subtotals, data_passer);
       g_slist_foreach(property->expense_accounts, make_subtotals, data_passer);
       generate_property_report(property, data_passer); */
}