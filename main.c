#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers.h"

static int callback(void *user_data, int argc, char **argv, char **azColName) {
    /*   int *gag = (int *)user_data;
      (*gag) ++;
      printf("The value of gag is %d\n",*gag);
     int i;
     for(i = 0; i<argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
     }
     printf("\n"); */
    return 0;
}

int main(int argc, char *argv[]) {

    

  

 /*    sql = "SELECT guid,name,description FROM accounts WHERE parent_guid = \"09f67b1fbae223eca818ba617edf1b3c\";";

    int barf = 0;

    rc = sqlite3_exec(data_passer->db, sql, callback, &barf, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Table created successfully\n");
    } */
    Data_passer *data_passer = setup();

    g_slist_foreach (data_passer->properties,make_pl_report,data_passer);


  /*   for (int i = 0; i < g_slist_length(properties); i++) {
        gpointer *barf = g_slist_nth_data(properties, i);
        Property *omg = (Property *)barf;

        g_print("guid %s, description %s\n", omg->guid, omg->description);
    }

    g_slist_foreach(properties, accumulate_income, data_passer);
   sqlite3_close(data_passer->db); */
    return 0;
}