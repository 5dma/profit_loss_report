#include <gtk/gtk.h>
#include "headers.h"

void button_choose_clicked(GtkButton *button, gpointer user_data) {
Data_passer *data_passer = (Data_passer *)user_data;
        /* Go make the report. */
    make_pl_report(data_passer);
}