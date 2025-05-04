#include "app_base.h"

void on_destroy(GtkWidget *widget, gpointer data)
{
    GUIApp_t *app = (GUIApp_t *)data;
    app->window = NULL;
}