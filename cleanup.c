#include <stdio.h>

#include "headers.h"



void cleanup(GtkButton *btn_exit, Data_passer *data_passer) {
    fclose(data_passer->output_file);
    sqlite3_close(data_passer->db);

    g_free(data_passer->start_date);
    g_free(data_passer->end_date);

    g_free(data_passer);
}