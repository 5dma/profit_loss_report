#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers.h"

static int has_children(void *user_data, int argc, char **argv, char **azColName) {
    Data_passer *data_passer = (Data_passer *)user_data;

    data_passer->number_of_children = atoi(argv[0]);

//    g_print("The number of children is %d\n", data_passer->number_of_children);
    return (0);
}

static int build_tree(void *user_data, int argc, char **argv, char **azColName) {
    Data_passer *data_passer = (Data_passer *)user_data;
    gchar *local_description;
    if (argv[2] == NULL) {
        g_print("%s, %s\n", argv[0], argv[1]);
    } else {
        g_print("%s, %s, %s\n", argv[0], argv[1], argv[2]);
    }
    char sql[1000];
    gint num_bytes = g_snprintf(sql, 1000, "SELECT COUNT(*) FROM accounts WHERE parent_guid = \"%s\";", argv[0]);
  //  g_print("%s\n", sql);
    int rc;
    char *zErrMsg = 0;

    rc = sqlite3_exec(data_passer->db, sql, has_children, data_passer, &zErrMsg);

    if (data_passer->number_of_children > 0) {
        char child_sql[1000];
        gint num_bytes = g_snprintf(child_sql, 1000, "SELECT guid,name,description FROM accounts WHERE parent_guid = \"%s\";", argv[0]);
     //   g_print("%s\n", child_sql);
        rc = sqlite3_exec(data_passer->db, child_sql, build_tree, data_passer, &zErrMsg);

        if (rc != SQLITE_OK) {
            g_print("SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        } else {
  //          g_print("Everything is good\n");
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    /* Initialize data, primarily by reading the last saved JSON file. */
    Data_passer *data_passer = setup();

    const gchar *sql = "SELECT guid,name,description FROM accounts WHERE parent_guid = \"3b7d34a311409d76e3b83c7a575b02e1\";";
    int rc;
    char *zErrMsg = 0;

    rc = sqlite3_exec(data_passer->db, sql, build_tree, data_passer, &zErrMsg);

    if (rc != SQLITE_OK) {
        g_print("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
   //     g_print("Everything is good\n");
    }

    /* Go make the report. */
    make_pl_report(data_passer);

    /* Free memory, close file handles. */
    cleanup(data_passer);

    return 0;
}