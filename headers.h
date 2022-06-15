#include <gtk/gtk.h>
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


GSList *setup (sqlite3 *db);

#endif
