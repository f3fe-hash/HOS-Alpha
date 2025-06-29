#ifndef __APP_BASE_H__
#define __APP_BASE_H__

#include <gtk/gtk.h>
#include <cairo.h>
#include "root.h"
#include "types.h"

// Gtk CSS file
#define GTK_STYLE_CSS        "etc/textures/style.css"

// Textures
#define GTK_TEXTURES            "etc/textures/"
#define GTK_TEXTURE_APP_GEAR    "etc/textures/gear.png"
#define GTK_TEXTURE_TERMINAL    "etc/textures/terminal.png"
#define GTK_TEXTURE_FILE_SELECT "etc/textures/file.png"
#define GTK_TEXTURE_NETWATCH    "etc/textures/netwatch.png"

typedef struct
{
    GtkWindow*  window;
    GtkWidget** widgets;

    int(*activate)(GtkApplication *, gpointer); // Main app function
} GUIApp_t;

void on_destroy(GtkWidget *widget, gpointer data);

#endif