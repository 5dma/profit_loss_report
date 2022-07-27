#include <math.h>
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
        *subtotal = g_ascii_strtod(argv[1], NULL);
    }
    return 0;
}

/**
 * Formats a passed float number into a familiar currency value. For example, takes 51003 and formats it into 51,003.00.
 * @param number Any gfloat less than 999999.99
 * @return The formatted string.
 */
gchar *comma_formatted_amount(gfloat number) {
    //    gfloat amount = *number;
    gchar formatted_amount[100];
    int num;
    if (number < 1000) {
        num = g_snprintf(formatted_amount, 11, "%.2f", number);
    } else {
        gdouble first_group = floor(number / 1000);
        gfloat second_group = number - (first_group * 1000);
        num = g_snprintf(formatted_amount, sizeof(formatted_amount), "%.0f,%06.2f", first_group, second_group);
    }
    return g_strdup(formatted_amount);
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
        char error_message[1000];
        gint num_bytes = g_snprintf(error_message, 1000, "SQLite error: %s", sqlite3_errmsg(data_passer->db));
        gtk_statusbar_pop(GTK_STATUSBAR(data_passer->status_bar), data_passer->status_bar_context);
        gtk_statusbar_push(GTK_STATUSBAR(data_passer->status_bar), data_passer->status_bar_context, error_message);
        data_passer->error_condition = SQLITE_SELECT_FAILURE;
        sqlite3_free(zErrMsg);
    }

    /* Add the subtotal to the total revenue or total expense for the current property. */
    if (data_passer->subtotaling_revenues) {
        data_passer->total_revenues += subtotal;
    } else {
        data_passer->total_expenses += subtotal;
    }
    /* Print the subtotal for the current account. */

    gchar *formatted_subtotal = comma_formatted_amount(subtotal);
    fprintf(data_passer->output_file, ACCOUNT_REPORT, description, formatted_subtotal);
    g_free(formatted_subtotal);

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
        gtk_statusbar_pop(GTK_STATUSBAR(data_passer->status_bar), data_passer->status_bar_context);
        gtk_statusbar_push(GTK_STATUSBAR(data_passer->status_bar), data_passer->status_bar_context, "No properties in report tree, no report generated");
        data_passer->error_condition = SQLITE_SELECT_FAILURE;
        return;
    }

    GtkTreeIter income_expense_iter;
    GtkTreeIter line_item_iter;
    do {
        gchar *description; /* Memory freed in while statement */
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

        gchar *formatted_income_total = comma_formatted_amount(data_passer->total_revenues);
        fprintf(data_passer->output_file, INCOME_TOTAL, formatted_income_total);
        g_free(formatted_income_total);

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
        gchar *formatted_expense_total = comma_formatted_amount(data_passer->total_expenses);
        fprintf(data_passer->output_file, INCOME_TOTAL, formatted_expense_total);
        g_free(formatted_expense_total);

        gchar *formatted_net_income_total = comma_formatted_amount(data_passer->total_revenues - data_passer->total_expenses);
        fprintf(data_passer->output_file, NET_INCOME, formatted_net_income_total);
        g_free(formatted_net_income_total);

        fputs("</table>\n", data_passer->output_file);
        g_free(description);
    } while (gtk_tree_model_iter_next(tree_model, &report_store_top_iter));
}

/**
 * Callback fired when user clicks the generate button.
 * @param button Pointer to the generate button.
 * @param user_data Pointer to a Data_passer struct.
 */
void make_pl_report(GtkButton *button, gpointer user_data) {
    Data_passer *data_passer = (Data_passer *)user_data;

    const gchar *report_start = "<!DOCTYPE HTML>\n<html lang=\"en\">\n<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n<title>P &amp; L Rental Properties</title>\n<link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.0.2/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-EVSTQN3/azprG1Anm3QDgpJLIm9Nao0Yz1ztcQTwFspd3yD65VohhpuuCOmLASjC\" crossorigin=\"anonymous\">\n<style>\ntd:nth-child(2) {text-align: right;}\n.single_underline {text-decoration: underline;}\n.double_underline {border-bottom:double black;}\n.left_indent {padding-left: 10px}\nh3 {margin-top: 50px;}\n</style>\n</head>\n<body class=\"p-4\">\n";

    const gchar *report_heading = "<h2 class=\"text-center\">Profit and Loss Report, Rental Properties</h3>\n";
    const gchar *report_end = "</body>\n</html>";

    data_passer->output_file = fopen(data_passer->output_file_name, "w");

    if (data_passer->output_file == NULL) {
        gtk_statusbar_pop(GTK_STATUSBAR(data_passer->status_bar), data_passer->status_bar_context);
        gtk_statusbar_push(GTK_STATUSBAR(data_passer->status_bar), data_passer->status_bar_context, "Could not open the output file for writing.");
        data_passer->error_condition = REPORT_GENERATION_FAILURE;
        return;
    }

    fputs(report_start, data_passer->output_file);
    fputs(report_heading, data_passer->output_file);

    /* Extract printer-friendly start and end dates, send to output. */
    gchar *start_date = g_utf8_substring(data_passer->start_date, 0, 10);
    gchar *end_date = g_utf8_substring(data_passer->end_date, 0, 10);
    fprintf(data_passer->output_file, DATE_RANGE, start_date, end_date);
    g_free(end_date);
    g_free(start_date);

    make_property_report(data_passer);

    fputs(report_end, data_passer->output_file);

    fclose(data_passer->output_file);
}
