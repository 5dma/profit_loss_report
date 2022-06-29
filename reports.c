#include "headers.h"

void add_property_to_store(gpointer data, gpointer user_data) {
    Property *property = (Property *)data;
    Data_passer *data_passer = (Data_passer *)user_data;
    GtkTreeIter iter;
    gtk_tree_store_append(data_passer->reports_store, &iter, NULL);
    gtk_tree_store_set(data_passer->reports_store, &iter, 0, property->description,-1);
}

void read_reports_tree(Data_passer *data_passer) {
    data_passer->reports_store = gtk_tree_store_new(1, G_TYPE_STRING) ;
    g_slist_foreach(data_passer->properties, add_property_to_store, data_passer);
   
}