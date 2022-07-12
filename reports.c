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

    gtk_tree_store_clear(data_passer->reports_store);

    read_properties_into_reports_store(data_passer);
    g_print("Reverted\n");
}

void save_report_tree(GtkButton *button, gpointer user_data) {
    Data_passer *data_passer = (Data_passer *)user_data;
    GtkTreeStore *reports_store = data_passer->reports_store;
    GtkTreeModel *tree_model = GTK_TREE_MODEL(data_passer->reports_store);
    GtkTreeIter report_store_top_iter;
    gboolean found_top_iter = gtk_tree_model_get_iter_first(tree_model, &report_store_top_iter);

    if (!found_top_iter) {
        g_print("No properties in report tree, no save performed\n");
        return;
    }

    JsonBuilder *builder = json_builder_new();
    json_builder_begin_object(builder);

    json_builder_set_member_name(builder, "start_date");
    if (data_passer->start_date == NULL) {
        json_builder_add_null_value(builder);
    } else {
        json_builder_add_string_value(builder, data_passer->start_date);
    }

    json_builder_set_member_name(builder, "end_date");
    if (data_passer->end_date == NULL) {
        json_builder_add_null_value(builder);
    } else {
        json_builder_add_string_value(builder, data_passer->end_date);
    }

    json_builder_set_member_name(builder, "properties");

    json_builder_begin_array(builder);

    gchararray guid;
    gchararray description;
    JsonNode *barf;
    gchar *code = (gchar *)g_malloc0(100);
    GtkTreeIter income_expense_iter;
    GtkTreeIter line_item_iter;
    do {
        gtk_tree_model_get(tree_model, &report_store_top_iter, GUID_REPORT, &guid, DESCRIPTION_REPORT, &description, -1);
        json_builder_begin_object(builder);
        json_builder_set_member_name(builder, "code");

        gchar *first_space = g_strstr_len(description, -1, " ");
        gint num_chars_to_copy = first_space - description;

        memset(code, '\0', 100);
        for (gint i = 0; i < num_chars_to_copy; i++) {
            code[i] = description[i];
        }

        barf = json_node_new(JSON_NODE_VALUE);
        json_node_set_string(barf, code);
        json_builder_add_value(builder, barf);

        json_builder_set_member_name(builder, "guid");
        barf = json_node_new(JSON_NODE_VALUE);
        json_node_set_string(barf, guid);
        json_builder_add_value(builder, barf);

        json_builder_set_member_name(builder, "income_accounts");

        /* Get iter for "Revenue" for current property. */
        gboolean found_revenue_header = gtk_tree_model_iter_nth_child(tree_model, &income_expense_iter, &report_store_top_iter, INCOME);
        if (found_revenue_header) {
            /* Check if the "Revenue" iter has individual revenue line items. */
            gboolean found_revenue_entries = gtk_tree_model_iter_has_child(tree_model, &income_expense_iter);
            if (found_revenue_entries) {
                json_builder_begin_array(builder);
                /* For each revenue line item, print its subtotal over the date range and accumulate. */
                gtk_tree_model_iter_children(tree_model, &line_item_iter, &income_expense_iter);
                do {
                    gtk_tree_model_get(GTK_TREE_MODEL(data_passer->reports_store), &line_item_iter, GUID_REPORT, &guid, -1);
                    barf = json_node_new(JSON_NODE_VALUE);
                    json_node_set_string(barf, guid);
                    json_builder_add_value(builder, barf);
                } while (gtk_tree_model_iter_next(tree_model, &line_item_iter));
                json_builder_end_array(builder);
            } else {
                json_builder_add_null_value(builder);
            }
        }

        json_builder_set_member_name(builder, "expense_accounts");

        /* Get iter for "Revenue" for current property. */
        gboolean found_expense_header = gtk_tree_model_iter_nth_child(tree_model, &income_expense_iter, &report_store_top_iter, EXPENSE);
        if (found_expense_header) {
            /* Check if the "Revenue" iter has individual revenue line items. */
            gboolean found_revenue_entries = gtk_tree_model_iter_has_child(tree_model, &income_expense_iter);
            if (found_revenue_entries) {
                json_builder_begin_array(builder);
                /* For each revenue line item, print its subtotal over the date range and accumulate. */
                gtk_tree_model_iter_children(tree_model, &line_item_iter, &income_expense_iter);
                do {
                    gtk_tree_model_get(GTK_TREE_MODEL(data_passer->reports_store), &line_item_iter, GUID_REPORT, &guid, -1);
                    barf = json_node_new(JSON_NODE_VALUE);
                    json_node_set_string(barf, guid);
                    json_builder_add_value(builder, barf);
                } while (gtk_tree_model_iter_next(tree_model, &line_item_iter));
                json_builder_end_array(builder);

            } else {
                json_builder_add_null_value(builder);
            }
        }

        json_builder_end_object(builder);
    } while (gtk_tree_model_iter_next(tree_model, &report_store_top_iter));
    json_builder_end_array(builder);

    json_builder_end_object(builder);

    JsonGenerator *generator = json_generator_new();
    json_generator_set_pretty(generator, TRUE);
    JsonNode *root = json_builder_get_root(builder);
    json_generator_set_root(generator, root);
    gchar *str = json_generator_to_data(generator, NULL);
    GError *gerror = NULL;
    gchar *output_file = g_build_filename(g_get_home_dir(), ".profit_loss/accounts.json", NULL);

    gboolean wrote_json_file = json_generator_to_file(generator, output_file, &gerror);
    g_free(code);
    json_node_free(root);
    g_object_unref(generator);
    g_object_unref(builder);
    g_free(output_file);

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