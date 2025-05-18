#include "gtk_main.h"
#include "gtk_utils.c"

GtkApp* gtk = NULL;

void initgtk()
{
    gtk = mp_alloc(mpool_, sizeof(GtkApp));

    gtk->apps = mp_alloc(mpool_, sizeof(GUIApp_t) * MAX_APPS);
    gtk->num_apps = 0;

    gtk->gtkapp = gtk_application_new(NULL, G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(gtk->gtkapp, "activate", G_CALLBACK(gtkactivate), NULL);

    // Filter out "--gui" or "-g" before passing to GTK
    int gtk_argc = __argc;
    char** gtk_argv = g_strdupv(__argv);

    for (int i = 1; i < gtk_argc; ++i)
    {
        if (strcmp(gtk_argv[i], "--gui") == 0 || strcmp(gtk_argv[i], "-g") == 0)
        {
            for (int j = i; j < gtk_argc - 1; ++j)
                gtk_argv[j] = gtk_argv[j + 1];
            gtk_argv[--gtk_argc] = NULL;
            i--; // Check this position again
        }
    }

    gtk->status = g_application_run(G_APPLICATION(gtk->gtkapp), gtk_argc, gtk_argv);
    g_object_unref(gtk->gtkapp);
    g_strfreev(gtk_argv);
}

static void gtkactivate(GtkApplication* app, gpointer user_data)
{
    GtkWidget* window = gtk_application_window_new(app);
    gtk->window = GTK_WINDOW(window);
    gtk_window_set_title(GTK_WINDOW(window), "HOS Desktop");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    GtkWidget* main_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_hbox);

    // LEFT COLUMN (gear icon + other controls)
    GtkWidget* left_column = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_vexpand(left_column, TRUE);
    gtk_widget_set_size_request(left_column, 150, 0);
    gtk_box_pack_start(GTK_BOX(main_hbox), left_column, FALSE, FALSE, 0);

    // Terminal and Files icons at the top
    GdkPixbuf* terminal_pixbuf = gdk_pixbuf_new_from_file_at_scale(GTK_TEXTURE_TERMINAL, 48, 48, TRUE, NULL);
    GdkPixbuf* files_pixbuf = gdk_pixbuf_new_from_file_at_scale(GTK_TEXTURE_FILE_SELECT, 48, 48, TRUE, NULL);
    if (!terminal_pixbuf || !files_pixbuf)
    {
        g_warning("Failed to load terminal or files icon.");
        return;
    }

    GtkWidget* terminal_icon = gtk_image_new_from_pixbuf(terminal_pixbuf);
    GtkWidget* files_icon = gtk_image_new_from_pixbuf(files_pixbuf);

    GtkWidget* terminal_button = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(terminal_button), terminal_icon);
    gtk_button_set_relief(GTK_BUTTON(terminal_button), GTK_RELIEF_NONE);

    GtkWidget* files_button = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(files_button), files_icon);
    gtk_button_set_relief(GTK_BUTTON(files_button), GTK_RELIEF_NONE);

    gtk_box_pack_start(GTK_BOX(left_column), terminal_button, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(left_column), files_button, FALSE, FALSE, 4);

    // Connect actions to launch apps
    static GUIApp_t terminal_app = {.activate = gtk_terminal_app_activate};
    static GUIApp_t files_app = {.activate = files_app_activate};

    g_signal_connect(terminal_button, "clicked", G_CALLBACK(launch_gui_app), &terminal_app);
    g_signal_connect(files_button, "clicked", G_CALLBACK(launch_gui_app), &files_app);

    // App gear icon
    GError* error = NULL;
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file_at_scale(GTK_TEXTURE_APP_GEAR, 48, 48, TRUE, &error);
    if (!pixbuf)
    {
        g_warning("Failed to load app gear icon: %s", error->message);
        g_error_free(error);
        return;
    }

    GtkWidget* apps_icon = gtk_image_new_from_pixbuf(pixbuf);
    GtkWidget* apps_button = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(apps_button), apps_icon);
#ifndef GTK_RELIEF_NONE
#define GTK_RELIEF_NONE (GtkReliefStyle)2
#endif
    gtk_button_set_relief(GTK_BUTTON(apps_button), GTK_RELIEF_NONE);

    // Add gear button to the bottom of the left column
    gtk_box_pack_end(GTK_BOX(left_column), apps_button, FALSE, FALSE, 4);

    // Popover menu
    GtkWidget* popover = gtk_popover_new(apps_button);
    GtkWidget* menu_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_set_border_width(GTK_CONTAINER(menu_box), 10);
    gtk_container_add(GTK_CONTAINER(popover), menu_box);
    g_signal_connect(apps_button, "clicked", G_CALLBACK(on_apps_button_clicked), popover);

    // Vertical line separator
    GtkWidget* line_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(line_area, 2, -1);
    gtk_widget_set_vexpand(line_area, TRUE);
    gtk_box_pack_start(GTK_BOX(main_hbox), line_area, FALSE, FALSE, 0);
    g_signal_connect(line_area, "draw", G_CALLBACK(draw_line), NULL);

    // MAIN AREA
    GtkWidget* main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(main_hbox), main_vbox, TRUE, TRUE, 0);

    GtkWidget* desktop_area = gtk_label_new("Desktop");
    gtk_widget_set_vexpand(desktop_area, TRUE);
    gtk_box_pack_start(GTK_BOX(main_vbox), desktop_area, TRUE, TRUE, 0);

    // Bottom bar (now just empty or for future use)
    GtkWidget* bottom_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), bottom_box, FALSE, FALSE, 0);

    // Optional CSS
    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, GTK_STYLE_CSS, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    gtk_widget_show_all(window);
}

static void on_apps_button_clicked(GtkWidget* button, gpointer user_data)
{
    GtkWidget* apps_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(apps_window), "Available Apps");
    gtk_window_set_default_size(GTK_WINDOW(apps_window), 200, 150);
    gtk_window_set_transient_for(GTK_WINDOW(apps_window), gtk->window);
    gtk_window_set_modal(GTK_WINDOW(apps_window), TRUE);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(apps_window), vbox);

    // App data
    static GUIApp_t terminal_app = {.activate = gtk_terminal_app_activate};
    static GUIApp_t files_app    = {.activate = files_app_activate};
    static GUIApp_t netwatch_app = {.activate = gtk_netwatch_app_activate};

    // Load 48x48 icons
    GdkPixbuf* terminal_pixbuf = gdk_pixbuf_new_from_file_at_scale(GTK_TEXTURE_TERMINAL, 48, 48, TRUE, NULL);
    GdkPixbuf* files_pixbuf = gdk_pixbuf_new_from_file_at_scale(GTK_TEXTURE_FILE_SELECT, 48, 48, TRUE, NULL);
    GdkPixbuf* netwatch_pixbuf = gdk_pixbuf_new_from_file_at_scale(GTK_TEXTURE_NETWATCH, 48, 48, TRUE, NULL);
    if (!terminal_pixbuf || !files_pixbuf || !netwatch_pixbuf)
    {
        g_warning("Failed to load one or more app icons.");
        return;
    }

    // Create icon widgets
    GtkWidget* terminal_icon = gtk_image_new_from_pixbuf(terminal_pixbuf);
    GtkWidget* files_icon = gtk_image_new_from_pixbuf(files_pixbuf);
    GtkWidget* netwatch_icon = gtk_image_new_from_pixbuf(netwatch_pixbuf);

    // Create image-only flat buttons
    GtkWidget* terminal_button = gtk_button_new();
    GtkWidget* files_button = gtk_button_new();
    GtkWidget* netwatch_button = gtk_button_new();

    // Set images
    gtk_button_set_image(GTK_BUTTON(terminal_button), terminal_icon);
    gtk_button_set_image(GTK_BUTTON(files_button), files_icon);
    gtk_button_set_image(GTK_BUTTON(netwatch_button), netwatch_icon);

    // Set button sizes (optional but consistent)
    gtk_widget_set_size_request(terminal_button, 32, 48);
    gtk_widget_set_size_request(files_button, 32, 48);
    gtk_widget_set_size_request(netwatch_button, 32, 48);

    // Remove button relief (flat appearance)
    gtk_button_set_relief(GTK_BUTTON(terminal_button), GTK_RELIEF_NONE);
    gtk_button_set_relief(GTK_BUTTON(files_button), GTK_RELIEF_NONE);
    gtk_button_set_relief(GTK_BUTTON(netwatch_button), GTK_RELIEF_NONE);

    // Style classes
    GtkStyleContext* ctx1 = gtk_widget_get_style_context(terminal_button);
    GtkStyleContext* ctx2 = gtk_widget_get_style_context(files_button);
    GtkStyleContext* ctx3 = gtk_widget_get_style_context(netwatch_button);
    gtk_style_context_add_class(ctx1, "flat-icon");
    gtk_style_context_add_class(ctx2, "flat-icon");
    gtk_style_context_add_class(ctx3, "flat-icon");

    // Add to layout
    gtk_box_pack_start(GTK_BOX(vbox), terminal_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), files_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), netwatch_button, FALSE, FALSE, 0);

    // Signal connections
    g_signal_connect(terminal_button, "clicked", G_CALLBACK(launch_gui_app), &terminal_app);
    g_signal_connect(files_button, "clicked", G_CALLBACK(launch_gui_app), &files_app);
    g_signal_connect(netwatch_button, "clicked", G_CALLBACK(launch_gui_app), &netwatch_app);

    g_signal_connect(terminal_button, "clicked", G_CALLBACK(close_apps_window), apps_window);
    g_signal_connect(files_button, "clicked", G_CALLBACK(close_apps_window), apps_window);
    g_signal_connect(netwatch_button, "clicked", G_CALLBACK(close_apps_window), apps_window);

    gtk_widget_show_all(apps_window);
}

void launch_gui_app(GtkWidget* widget, gpointer user_data)
{
    if (gtk->num_apps >= MAX_APPS)
    {
        g_warning("Maximum number of sub-apps reached.");
        return;
    }

    GUIApp_t* app = mp_alloc(mpool_, sizeof(GUIApp_t));
    app = (GUIApp_t* )user_data;

    // If already running, bring to front
    if (app->window != NULL)
    {
        gtk_window_present(app->window);
        return;
    }

    // Track the app before activation
    gtk->apps[gtk->num_apps++] =* app;

    // Call the app's activate function to create its window
    if (app->activate)
        app->activate(gtk->gtkapp, app);
}

// Example "Files" app
static int files_app_activate(GtkApplication* app, gpointer user_data)
{
    GUIApp_t* gapp = mp_alloc(mpool_, sizeof(GUIApp_t));
    gapp = (GUIApp_t* )user_data;

    GtkWidget* window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Files");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

    GtkWidget* label = gtk_label_new("File Manager App");
    gtk_container_add(GTK_CONTAINER(window), label);

    gtk_widget_show_all(window);
    gapp->window = GTK_WINDOW(window);

    return 0;
}
