#include "gtk_terminal.h"

const char *PROMPT = "hos@HOS:~$ ";

static void append_prompt(TerminalApp *app)
{
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(app->buffer, &end);
    gtk_text_buffer_insert(app->buffer, &end, PROMPT, -1);

    // Set input mark here
    app->input_mark = gtk_text_buffer_create_mark(app->buffer, "input_start", &end, TRUE);
}

void append_to_terminal(GtkTextBuffer *buffer, const char *text)
{
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_insert(buffer, &end, text, -1);
}

void execute_command(TerminalApp *app, const char *input)
{
    gtk_execute((char *)input, app->buffer);
    append_prompt(app); // Add a new prompt after the output
}

void gtk_execute(char *input, GtkTextBuffer *buffer)
{
    char line[512];
    snprintf(line, sizeof(line), "\n");
    append_to_terminal(buffer, line);

    // Clone and tokenize input
    char cmdline[512];
    strncpy(cmdline, input, sizeof(cmdline));
    cmdline[sizeof(cmdline) - 1] = '\0';

    char *tokens[256];
    int i = 0;

    char *token = strtok(cmdline, " ");
    while (token && i < 255)
    {
        tokens[i++] = token;
        token = strtok(NULL, " ");
    }
    tokens[i] = NULL;

    if (i == 0)
        return;

    const char *cmd = tokens[0];
    char out[2048]; // larger buffer for things like hpm

    if (strcmp(cmd, "clear") == 0)
    {
        gtk_text_buffer_set_text(buffer, "", -1);
        return;
    }

    else if (strcmp(cmd, "echo") == 0)
    {
        out[0] = '\0';
        for (int j = 1; tokens[j]; j++)
        {
            strcat(out, tokens[j]);
            strcat(out, " ");
        }
        strcat(out, "\n");
        append_to_terminal(buffer, out);
    }

    else if (strcmp(cmd, "exit") == 0)
    {
        gtk_main_quit();
    }

    else if (strcmp(cmd, "version") == 0)
    {
        snprintf(out, sizeof(out), "HOS %s\n", HOS_VERSION);
        append_to_terminal(buffer, out);
    }

    else if (strcmp(cmd, "help") == 0)
    {
        FILE *file = fopen(HELP_FILE, "r");
        if (!file)
        {
            append_to_terminal(buffer, "Error opening help file.\n");
            return;
        }
        while (fgets(out, sizeof(out), file))
            append_to_terminal(buffer, out);
        append_to_terminal(buffer, "\n");
        fclose(file);
    }

    else if (strcmp(cmd, "hash") == 0)
    {
        if (!tokens[1] || !tokens[2])
        {
            append_to_terminal(buffer, "Usage: hash <string> <hash-type>\n");
            return;
        }

        const char *input = tokens[1];
        const char *algo = tokens[2];
        const unsigned char *raw_hash = NULL;
        size_t hash_len = 0;

        if (strcmp(algo, "sha1") == 0)
        {
            raw_hash = sha1((byte *)input, strlen(input));
            hash_len = 20;
        }
        else if (strcmp(algo, "sha256") == 0)
        {
            raw_hash = sha256((byte *)input, strlen(input));
            hash_len = 32;
        }
        else if (strcmp(algo, "sha512") == 0)
        {
            raw_hash = sha512((byte *)input, strlen(input));
            hash_len = 64;
        }
        else
        {
            snprintf(out, sizeof(out), "Unsupported hash type: %s\n", algo);
            append_to_terminal(buffer, out);
            return;
        }

        snprintf(out, sizeof(out), "Hash (%s): %s\n", algo, to_hex_string(raw_hash, hash_len));
        append_to_terminal(buffer, out);
    }

    else if (strcmp(cmd, "setport") == 0)
    {
        if (!tokens[1])
        {
            append_to_terminal(buffer, "Usage: setport <port>\n");
            return;
        }

        port = atoi(tokens[1]);
        snprintf(out, sizeof(out), "Port set to %d\n", port);
        append_to_terminal(buffer, out);
    }

    else if (strcmp(cmd, "ping") == 0)
    {
        if (!tokens[1] || !tokens[2] || !tokens[3] || !tokens[4])
        {
            append_to_terminal(buffer, "Usage: ping <packets> <packet size> <hostname/IP> <timeout (microseconds)>\n");
            return;
        }

        int packets = atoi(tokens[1]);
        int packet_size = atoi(tokens[2]);
        char *target = tokens[3];
        int timeout = atoi(tokens[4]);

        if (packets <= 0 || packet_size <= 0)
        {
            append_to_terminal(buffer, "Invalid packet count or size.\n");
            return;
        }

        if (timeout <= 2000)
            timeout = 10000;

        struct sockaddr_in dest_addr;
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(0);
        inet_pton(AF_INET, target, &dest_addr.sin_addr);

        ping(dest_addr.sin_addr.s_addr, packet_size, packets, timeout);
    }

    else if (strcmp(cmd, "hpm") == 0)
    {
        if (!tokens[1] || !tokens[2] || !tokens[3])
        {
            append_to_terminal(buffer, "Usage: hpm <command> <package> <version>\n");
            return;
        }

        const char *command = tokens[1];
        const char *name = tokens[2];
        const char *version = tokens[3];

        package_t *pkg = init_package();
        if (!pkg)
        {
            append_to_terminal(buffer, "Failed to allocate package.\n");
            return;
        }

        pkg->name = strdup(name);
        pkg->version = strdup(version);

        if (strcmp(command, "install") == 0)
            install_package(pkg);
        else if (strcmp(command, "download") == 0)
            download_package(pkg);
        else
        {
            snprintf(out, sizeof(out), "Unknown HPM command: %s\n", command);
            append_to_terminal(buffer, out);
        }

        free(pkg->name);
        free(pkg->version);
        free(pkg);
    }

    else
    {
        snprintf(out, sizeof(out), "Unknown command: %s\n", cmd);
        append_to_terminal(buffer, out);
    }
}

static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    TerminalApp *app = user_data;

    GtkTextIter mark_iter;
    gtk_text_buffer_get_iter_at_mark(app->buffer, &mark_iter, app->input_mark);

    GtkTextIter cursor_iter;
    GtkTextBuffer *buffer = app->buffer;
    gtk_text_buffer_get_iter_at_mark(buffer, &cursor_iter,
                                     gtk_text_buffer_get_insert(buffer));

    if (gtk_text_iter_compare(&cursor_iter, &mark_iter) < 0)
    {
        // Prevent editing before prompt
        return TRUE;
    }

    if (event->keyval == GDK_KEY_Return)
    {
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(buffer, &end);

        gchar *input = gtk_text_buffer_get_text(buffer, &mark_iter, &end, FALSE);
        execute_command(app, input);
        g_free(input);
        return TRUE;
    }

    return FALSE;
}

int gtk_terminal_app_activate(GtkApplication *app, gpointer data)
{
    TerminalApp *terminal_app = g_new0(TerminalApp, 1);

    terminal_app->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(terminal_app->window), "GTK Terminal");
    gtk_window_set_default_size(GTK_WINDOW(terminal_app->window), 800, 600);

    terminal_app->text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(terminal_app->text_view), TRUE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(terminal_app->text_view), GTK_WRAP_WORD_CHAR);
    gtk_widget_set_hexpand(terminal_app->text_view, TRUE);
    gtk_widget_set_vexpand(terminal_app->text_view, TRUE);

    terminal_app->buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(terminal_app->text_view));
    append_prompt(terminal_app);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), terminal_app->text_view);
    gtk_container_add(GTK_CONTAINER(terminal_app->window), scroll);

    g_signal_connect(terminal_app->text_view, "key-press-event", G_CALLBACK(on_key_press), terminal_app);

    gtk_widget_show_all(terminal_app->window);
}