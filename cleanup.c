#include <stdio.h>

#include "headers.h"

void free_account_list(gpointer data) {
    Account_summary *account_summary = (Account_summary *)data;
    g_free(account_summary->guid);
    g_free(account_summary->description);
}

void free_property_list(gpointer data, gpointer user_data) {
    Property *property = (Property *)data;
    g_free(property->guid);
    g_free(property->description);

    g_slist_free_full(property->income_accounts, free_account_list);
    g_slist_free_full(property->expense_accounts, free_account_list);
}

void cleanup(Data_passer *data_passer) {
    fclose(data_passer->output_file);
    sqlite3_close(data_passer->db);

    g_free(data_passer->start_date);
    g_free(data_passer->end_date);

    g_slist_foreach(data_passer->properties, free_property_list, NULL);
    g_slist_free(data_passer->properties);

    g_free(data_passer);
}