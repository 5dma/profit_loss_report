#include "headers.h"

static int total_up_income(void *user_data, int argc, char **argv, char **azColName) {
    GSList **properties = (GSList **)user_data;

    gchar *end_ptr;

    GTimeZone *time_zone = g_time_zone_new_local(); /* Need to instantiate this once */

    Transaction *transaction = g_new(Transaction, 1);

    transaction->split_guid = g_strdup(argv[0]);
    transaction->transaction_id = g_strdup(argv[1]);
    transaction->value = g_ascii_strtod(argv[4], &end_ptr);
    transaction->post_date = g_date_time_new_from_iso8601(argv[5], time_zone);
    g_free(time_zone);
    return 0;
}

void accumulate_income(gpointer data, gpointer user_data) {
    gchar *income_account = (gchar *)data;
    Data_passer *data_passer = (Data_passer *)user_data;

    gfloat total;

    int rc;
    char *sql;
    char *zErrMsg = 0;
  /*   sql =
        "SELECT splits.guid,tx_guid,value_num,value_denom, (value_num/value_denom) as real_number, post_date FROM splits JOIN transactions ON tx_guid=transactions.guid WHERE account_guid = \"7958e79e10b723bae9c3f5fef5234d24\" AND post_date > \"2022-01-01 00:00:00\";"; */

    gchar *format_string = "SELECT SUM(value_num/value_denom), accounts.description FROM splits JOIN transactions ON tx_guid=transactions.guid JOIN accounts ON account_guid = accounts.guid WHERE account_guid = \"%s\" AND post_date > \"%s\";";

    g_snprintf (sql, 1000, format_string,income_account,  data_passer->start_date);

    rc = sqlite3_exec(data_passer->db, sql, total_up_income, &(property->income_transactions), &zErrMsg);

    if (rc != SQLITE_OK) {
        g_print("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        g_print("Table created successfully\n");
    }

    gint i = g_slist_length(property->income_transactions);
    g_print("After it's all over,  length: %d\n", i);
}

void make_pl_report(gpointer data, gpointer user_data) {
    Property *property = (Property *)data;
  
    g_slist_foreach(property->income_accounts, accumulate_income, user_data);
}