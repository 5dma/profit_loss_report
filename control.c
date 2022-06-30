#include <gtk/gtk.h>
#include "headers.h"

void btn_report_clicked(GtkButton *button, gpointer user_data) {
Data_passer *data_passer = (Data_passer *)user_data;
        /* Go make the report. */
    make_pl_report(data_passer);
}


void account_tree_cursor_changed(GtkTreeView *tree_view_accounts, gpointer user_data) {
Data_passer *data_passer = (Data_passer *)user_data;
        /* Go make the report. */
    g_print("Changed the focus\n");
}