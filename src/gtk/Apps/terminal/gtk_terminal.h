#ifndef __GTK_TERMINAL_H__
#define __GTK_TERMINAL_H__

#include "gtk/Apps/app_base.h"
#include "gtk/Apps/NetWatch/gtk_netwatch.h"
#include "Commands/cmd_base.h"
#include <gtk/gtk.h>

/* GTK Terminal Application Structure */
typedef struct
{
    GtkWidget *window;
    GtkWidget *text_view;
    GtkTextBuffer *buffer;
    GtkTextMark *input_mark;
    GtkApplication *app;
} TerminalApp;

int  gtk_terminal_app_activate(GtkApplication *app, gpointer data);
void gtk_execute(TerminalApp *app, char *input, GtkTextBuffer *buffer);
void execute_command(TerminalApp *app, const char *input);
void append_to_terminal(GtkTextBuffer *buffer, const char *text);

#endif
