#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include "headers.h"

int main(int argc, char *argv[]) {

    /* Initialize data, primarily by reading the last saved JSON file. */
    Data_passer *data_passer = setup();

    /* Go make the report. */
    make_pl_report(data_passer);

    /* Free memory, close file handles. */
    cleanup(data_passer);

    return 0;
}