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
}

void make_pl_report(gpointer data, gpointer user_data) {
    Property *property = (Property *)data;

    g_slist_foreach(property->income_accounts, make_subtotals, user_data);
    g_slist_foreach(property->expense_accounts, make_subtotals, user_data);
    generate_property_report(property);
}