#include <json-glib/json-glib.h>

#include "headers.h"

void add_income_to_report_store(gpointer data, gpointer user_data) {
    Account_summary *account_summary = (Account_summary *)data;
    Iter_passer_reports *iter_passer_reports = (Iter_passer_reports *)user_data;
    GtkTreeIter child;
    gtk_tree_store_append(iter_passer_reports->reports_store, &child, &(iter_passer_reports->parent));
    // g_print("%s\n", account_summary->description);
    gtk_tree_store_set(iter_passer_reports->reports_store, &child, GUID_REPORT, account_summary->guid, DESCRIPTION_REPORT, account_summary->description, -1);
}

void add_property_to_store(gpointer data, gpointer user_data) {
    Property *property = (Property *)data;
    Data_passer *data_passer = (Data_passer *)user_data;

    GtkTreeIter parent_iter;
    gtk_tree_store_append(data_passer->reports_store, &parent_iter, NULL);
    gtk_tree_store_set(data_passer->reports_store, &parent_iter, GUID_REPORT, property->guid, DESCRIPTION_REPORT, property->description, -1);

    GtkTreeIter child_iter_income;
    gtk_tree_store_append(data_passer->reports_store, &child_iter_income, &parent_iter);
    gtk_tree_store_set(data_passer->reports_store, &child_iter_income, DESCRIPTION_REPORT, REVENUE, -1);

    Iter_passer_reports *iter_passer_reports = g_new(Iter_passer_reports, 1);
    iter_passer_reports->reports_store = data_passer->reports_store;
    iter_passer_reports->parent = child_iter_income;
    g_slist_foreach(property->income_accounts, add_income_to_report_store, iter_passer_reports);

    GtkTreeIter child_iter_expenses;
    gtk_tree_store_append(data_passer->reports_store, &child_iter_expenses, &parent_iter);
    gtk_tree_store_set(data_passer->reports_store, &child_iter_expenses, DESCRIPTION_REPORT, EXPENSES, -1);

    iter_passer_reports->parent = child_iter_expenses;
    g_slist_foreach(property->expense_accounts, add_income_to_report_store, iter_passer_reports);
}

void revert_report_tree(GtkButton *button, gpointer user_data) {
    Data_passer *data_passer = (Data_passer *)user_data;

    gtk_tree_store_clear (data_passer->reports_store);

    read_properties_into_reports_store(data_passer);
    g_print("Reverted\n");
}

/**
 * Performs a depth-first-search for a GUID inside the reports store. The logic is as follows.
 *
 * -# Receive the store, root iter, guid we are looking for, and the data passer.
 * -# If a match was found (`data_passer->is_guid_in_reports_tree == TRUE`), return.
 * -# Retrieve the guid at the current iter.
 * -# If the retrieved guid matches the target guid:
 *    -# A match was found  (`data_passer->is_guid_in_reports_tree = TRUE`).
 *    -# Return.
 * -# If the current iter has children:
 *    -# Get the first child iter.
 *    -# Recurse with  store, child iter, guid we are looking for, and the data passer
 * -# If the current iter has a sibling:
 *    -# Get the sibling iter.
 *    -# Recurse with  store, sibling iter, guid we are looking for, and the data passer
 * -# Return.
 */
void is_guid_in_reports_tree(GtkTreeStore *reports_store, GtkTreeIter current_iter, char *guid, Data_passer *data_passer) {
    if (data_passer->is_guid_in_reports_tree == TRUE) {
        return;
    }

    gchararray candidate_guid;
    gtk_tree_model_get(GTK_TREE_MODEL(reports_store), &current_iter, GUID_REPORT, &candidate_guid, -1);

    if (g_strcmp0(candidate_guid, guid) == 0) {
        data_passer->is_guid_in_reports_tree = TRUE;
        return;
    }
    GtkTreeIter child_iter;
    gboolean current_iter_has_children = gtk_tree_model_iter_children(GTK_TREE_MODEL(reports_store), &child_iter, &current_iter);
    if (current_iter_has_children == TRUE) {
        is_guid_in_reports_tree(reports_store, child_iter, guid, data_passer);
    }

    GtkTreeIter sibling_iter;
    gboolean current_iter_has_sibling = gtk_tree_model_iter_next(GTK_TREE_MODEL(reports_store), &current_iter);
    if (current_iter_has_sibling == TRUE) {
        is_guid_in_reports_tree(reports_store, current_iter, guid, data_passer);
    }

    return;
}

void read_properties_into_reports_store(Data_passer *data_passer) {
    /* Memory is freed at end of this function */
    gchar *input_file = g_build_filename(g_get_home_dir(), ".profit_loss/accounts.json", NULL);
    gboolean input_file_exists = g_file_test(input_file, G_FILE_TEST_EXISTS);
    if (input_file_exists) {
        GError *error = NULL;
        JsonParser *parser;
        JsonNode *root;

        /* Reference count decremented at end of this function. */
        parser = json_parser_new();
        json_parser_load_from_file(parser, input_file, &error);
        if (error) {
            g_print("Unable to parse `%s': %s\n", input_file, error->message);
            g_error_free(error);
        } else {
            JsonNode *root = json_parser_get_root(parser);
            JsonObject *root_obj = json_node_get_object(root);

            /* Pretty sure no need to free following string as it is a const. */
            const gchar *start_date_string = json_object_get_string_member(root_obj, "start_date");
            if (start_date_string != NULL) {
                data_passer->start_date = g_strdup(start_date_string);
            } else {
                data_passer->start_date = NULL;
            }

            /* Pretty sure no need to free following string as it is a const. */
            const gchar *end_date_string = json_object_get_string_member(root_obj, "end_date");
            if (end_date_string != NULL) {
                data_passer->end_date = g_strdup(end_date_string);
            } else {
                data_passer->end_date = NULL;
            }

            JsonArray *property_array = (JsonArray *)json_object_get_array_member(root_obj, "properties");

            guint len_properties = json_array_get_length(property_array);

            GtkTreeStore *reports_store = data_passer->reports_store;
            GtkTreeIter property_iter;
            gchararray description;
            for (int i = 0; i < len_properties; i++) {
                JsonObject *property_object = json_array_get_object_element(property_array, i);
                gtk_tree_store_append(reports_store, &property_iter, NULL);
                gchararray guid = g_strdup(json_object_get_string_member(property_object, "guid"));
                gchar description[1000];
                get_account_description(guid, description, data_passer);
                gtk_tree_store_set(reports_store, &property_iter, GUID_REPORT, guid, DESCRIPTION_REPORT, description, -1);

                add_accounts(data_passer, property_object, &property_iter, INCOME);
                add_accounts(data_passer, property_object, &property_iter, EXPENSE);
            }
        }
        g_object_unref(parser);
    } else {
        g_print("Input file does not exist.\n");
    }
    g_free(input_file);
}