#include "gtk_main.h"

// Drawing function for the vertical line
static gboolean draw_line(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
    // Set the line color (black for this example)
    cairo_set_source_rgb(cr, 0, 0, 0); // RGB (0,0,0) is black

    // Set the line width
    cairo_set_line_width(cr, 2); // 2px wide line

    // Draw a line from the top to the bottom of the widget
    cairo_move_to(cr, 0, 0);                                       // Start at the top-left
    cairo_line_to(cr, 0, gtk_widget_get_allocated_height(widget)); // End at the bottom-left

    // Stroke the path (actually draw the line)
    cairo_stroke(cr);

    return FALSE; // Returning FALSE allows the drawing to continue as usual
}

static void close_apps_window(GtkWidget *button, gpointer user_data)
{
    GtkWidget *apps_window = GTK_WIDGET(user_data);
    gtk_widget_destroy(apps_window); // Close the app selector window
}

static void on_subapp_destroy(GtkWidget *widget, gpointer user_data)
{
    GUIApp_t *app = mp_alloc(mpool_, sizeof(GUIApp_t));
    app = (GUIApp_t *)user_data;

    // Remove the app from the GtkApp apps array
    for (int i = 0; i < gtk->num_apps; ++i)
    {
        if (&gtk->apps[i] == app)
        {
            // Shift remaining apps left
            for (int j = i; j < gtk->num_apps - 1; ++j)
            {
                gtk->apps[j] = gtk->apps[j + 1];
            }
            gtk->num_apps--;
            break;
        }
    }
}

static void on_subapp_closed(GtkWidget *widget, gpointer user_data)
{
    GUIApp_t *app = mp_alloc(mpool_, sizeof(GUIApp_t));
    app = (GUIApp_t *)user_data;

    // Remove the app from the window list
    if (app->window)
    {
        gtk_window_close(app->window);
        app->window = NULL; // Reset window reference after closing
    }

    gtk->num_apps--; // Decrease the app count
}