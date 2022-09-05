#include <gtk/gtk.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers.h"

/**
 * @file main.c
 * @brief This GTK 3.0 application generates an income statement from user-specified GnuCash accounts.
 *
 * To generate this 
 * code documentation, run `doxygen Doxyfile`. The output is in the `html/` 
 * directory of this source code.
 */

/**
 * Starts the GTK loop.
 * @param argc Number of arguments from the command line, none in this case.
 * @param argv Arguments passed from the command line, none in this case.
*/
int main(int argc, char *argv[]) {

    GtkApplication *app = gtk_application_new(
        "net.lautman.customgnucashplreport",
        G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_app_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);
    return 0;
}