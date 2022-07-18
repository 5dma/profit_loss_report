#include <stdio.h>

#include "headers.h"

/**
 * @file accumulate.c
 * @brief Contains functions for generating the P&L report.
 *
 */

/**
 * Sqlite callback that returns the totals charged to a given account. The result is placed in a passed pointer.
 *
 * @param user_data Pointer to a `gfloat`. This pointer points to the account total.
 * @param argc Number of columns in sqlite result.
 * @param argv Array of pointers to the results of a query.
 * @param azColName Array of pointers to strings corresponding to result column names.
 * @see [One-Step Query Execution Interface](https://www.sqlite.org/c3ref/exec.html)
 */
static int total_up_income(void *user_data, int argc, char **argv, char **azColName) {
    gfloat *subtotal = (gfloat *)user_data;
    if (g_strcmp0(argv[0], "0") == 0) {
        *subtotal = 0;
    } else {
        gchar *end_ptr;
        *subtotal = g_ascii_strtod(argv[1], &end_ptr);
    }
    return 0;
}

/**
 * Totals the amounts charged to a given account.
 * @param income_expense_iter GtkTreeIter pointing to an expense or income account in the report tree.
 * @param data_passer Pointer to a Data_passer struct.
 */
void make_subtotals(GtkTreeIter income_expense_iter, Data_passer *data_passer) {
    gfloat subtotal;
    gchar *description;
    gchar *guid;
    int rc;
    char sql[1000];
    char *zErrMsg = 0;

    /* Retrieve the guid and description for the passed GtkTreeIter. */
    gtk_tree_model_get(GTK_TREE_MODEL(data_passer->reports_store), &income_expense_iter, GUID_REPORT, &guid, DESCRIPTION_REPORT, &description, -1);

    /* Make a database call to accumulate the amounts charged to the account. */
    gint num_bytes = g_snprintf(sql, 1000, SUM_OF_ACCOUNT_ACTIVITY, guid, guid, data_passer->start_date);
    rc = sqlite3_exec(data_passer->db, sql, total_up_income, &subtotal, &zErrMsg);

    if (rc != SQLITE_OK) {
        g_print("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        //      g_print("Table created successfully\n");
    }

    /* Add the subtotal to the total revenue or total expense for the current property. */
    if (data_passer->subtotaling_revenues) {
        data_passer->total_revenues += subtotal;
    } else {
        data_passer->total_expenses += subtotal;
    }
    /* Print the subtotal for the current account. */
    fprintf(data_passer->output_file, ACCOUNT_REPORT, description, subtotal);
    g_free(description);
    g_free(guid);
}

/**
 * Outputs a P&L report for each asset in the reports tree.
 *
 * @param data_passer Pointer to a Data_passer struct.
 */
void make_property_report(Data_passer *data_passer) {
    GtkTreeIter report_store_top_iter;

    GtkTreeModel *tree_model = GTK_TREE_MODEL(data_passer->reports_store);
    gboolean found_top_iter = gtk_tree_model_get_iter_first(tree_model, &report_store_top_iter);

    if (!found_top_iter) {
        g_print("No properties in report tree, no report generated\n");
        return;
    }

    gchararray description;
    GtkTreeIter income_expense_iter;
    GtkTreeIter line_item_iter;
    do {
        data_passer->total_revenues = 0;
        data_passer->total_expenses = 0;
        gtk_tree_model_get(tree_model, &report_store_top_iter, DESCRIPTION_REPORT, &description, -1);
        fprintf(data_passer->output_file, PROPERTY_HEADER, description);

        fputs(INCOME_HEADER, data_passer->output_file);
        data_passer->subtotaling_revenues = TRUE;

        /* print the revenue entries for the current property. */

        /* Get iter for "Revenue" for current property. */
        gboolean found_revenue_header = gtk_tree_model_iter_nth_child(tree_model, &income_expense_iter, &report_store_top_iter, INCOME);
        if (found_revenue_header) {
            /* Check if the "Revenue" iter has individual revenue line items. */
            gboolean found_revenue_entries = gtk_tree_model_iter_has_child(tree_model, &income_expense_iter);
            if (found_revenue_entries) {
                /* For each revenue line item, print its subtotal over the date range and accumulate. */
                gtk_tree_model_iter_children(tree_model, &line_item_iter, &income_expense_iter);
                do {
                    make_subtotals(line_item_iter, data_passer);
                } while (gtk_tree_model_iter_next(tree_model, &line_item_iter));
            }
        }
        fprintf(data_passer->output_file, INCOME_TOTAL, data_passer->total_revenues);

        fputs(EXPENSE_HEADER, data_passer->output_file);
        data_passer->subtotaling_revenues = FALSE;
        gboolean found_expenses_header = gtk_tree_model_iter_nth_child(tree_model, &income_expense_iter, &report_store_top_iter, EXPENSE);
        if (found_expenses_header) {
            gboolean found_expense_entries = gtk_tree_model_iter_has_child(tree_model, &income_expense_iter);
            if (found_expense_entries) {
                gtk_tree_model_iter_children(tree_model, &line_item_iter, &income_expense_iter);
                do {
                    make_subtotals(line_item_iter, data_passer);
                } while (gtk_tree_model_iter_next(tree_model, &line_item_iter));
            }
        }
        fprintf(data_passer->output_file, EXPENSE_TOTAL, data_passer->total_expenses);
        fprintf(data_passer->output_file, NET_INCOME, data_passer->total_revenues - data_passer->total_expenses);
        fputs("</table>\n", data_passer->output_file);
    } while (gtk_tree_model_iter_next(tree_model, &report_store_top_iter));
}

/**
 * Callback fired when user clicks on the generate button.
 * @param button Pointer to the generate button.
 * @param user_data Pointer to a Data_passer struct.
 */
void make_pl_report(GtkButton *button, gpointer user_data) {
    Data_passer *data_passer = (Data_passer *)user_data;

    gchar *report_start = "<!DOCTYPE HTML>\n<html lang=\"en\">\n<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n<title>P &amp; L Rental Properties</title>\n<link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.0.2/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-EVSTQN3/azprG1Anm3QDgpJLIm9Nao0Yz1ztcQTwFspd3yD65VohhpuuCOmLASjC\" crossorigin=\"anonymous\">\n<style>\ntd:nth-child(2) {text-align: right;}\n.single_underline {text-decoration: underline;}\n.double_underline {border-bottom:double black;}\n.left_indent {padding-left: 10px}\nh3 {margin-top: 50px;}\n</style>\n</head>\n<body class=\"p-4\">\n";

    gchar *report_end = "</body>\n</html>";

    data_passer->output_file = fopen("/tmp/property_pl.html", "w");

    if (data_passer->output_file == NULL) {
        g_print("Cannot create the report, exiting\n");
        return;
    }

    fputs(report_start, data_passer->output_file);

    make_property_report(data_passer);

    fputs(report_end, data_passer->output_file);

    fclose(data_passer->output_file);
}