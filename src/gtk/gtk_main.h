#ifndef __GTK_MAIN_H__
#define __GTK_MAIN_H__

#include <gtk/gtk.h>
#include <cairo.h>

#include "../root.h"
#include "Apps/app_base.h"
#include "Apps/terminal/gtk_terminal.h"
#include "Apps/NetWatch/gtk_netwatch.h"

// Limit on maximum sub-apps open at once
#define MAX_APPS 10

// Main application for gtk, main window, sub-apps
typedef struct
{
    GtkApplication* gtkapp;
    GtkWindow* window;
    GUIApp_t* apps;

    int num_apps;
    int status;
} GtkApp;

extern GtkApp* gtk;

static void on_apps_button_clicked(GtkWidget *button, gpointer user_data);
static void gtkactivate(GtkApplication* app, gpointer user_data);

static int files_app_activate(GtkApplication *app, gpointer user_data);
static void launch_gui_app(GtkWidget *widget, gpointer user_data);
static void close_apps_window(GtkWidget *button, gpointer user_data);
static void on_subapp_closed(GtkWidget *widget, gpointer user_data);

static gboolean draw_line(GtkWidget *widget, cairo_t *cr, gpointer user_data);

void initgtk();

#endif