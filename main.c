#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers.h"

int main(int argc, char *argv[]) {


    Data_passer *data_passer = setup();

    make_pl_report(data_passer);
    fclose(data_passer->output_file);
    return 0;
}