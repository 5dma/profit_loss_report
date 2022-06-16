#include <glib-2.0/glib.h>
#include <sqlite3.h>
#ifndef __HEADER
#define __HEADER

typedef struct { 
    gchar *description;
    gdouble *amount;
} Line_Item;    

typedef struct {
    gchar *guid;
    gchar *description;
    GSList *income;
    GSList *expenses;
} Property;

typedef struct {
    sqlite3 *db;
} Data_passer;

GSList *setup (sqlite3 *db);

#endif
