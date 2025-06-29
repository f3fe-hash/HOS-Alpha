#include "gtk_netwatch.h"

// Globals (defined here)
IPEntry src_ip_entries[MAX_IPS];
int src_ip_count = 0;

IPEntry dst_ip_entries[MAX_IPS];
int dst_ip_count = 0;

PortEntry src_ports[MAX_PORTS];
int src_port_count = 0;

PortEntry dst_ports[MAX_PORTS];
int dst_port_count = 0;

NetWatchApp* nwapp = NULL;

GtkListStore* store_src_ips = NULL;
GtkListStore* store_dst_ips = NULL;
GtkListStore* store_src_ports = NULL;
GtkListStore* store_dst_ports = NULL;
GtkListStore* store_recent_packets = NULL;

GtkWidget* overview_grid = NULL;
GtkWidget* lbl_total_packets = NULL;
GtkWidget* lbl_total_bytes = NULL;
GtkWidget* lbl_tcp = NULL;
GtkWidget* lbl_udp = NULL;
GtkWidget* lbl_icmp = NULL;
GtkWidget* lbl_other = NULL;
GtkWidget* lbl_overview = NULL;

GtkWidget* protocol_pie_chart = NULL;

GtkWidget* packet_list = NULL;

pthread_t capture_thread;
volatile int capturing = 1;
volatile int thread_running = 0;

int total_packets = 0;
int total_bytes = 0;
int tcp_count = 0, udp_count = 0, icmp_count = 0, other_count = 0;

Packet recent_packets[MAX_RECENT_PACKETS];
int packet_count = 0;

// Add mutex for thread safety (optional but recommended)
pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;

gboolean draw_protocol_pie(GtkWidget* widget, cairo_t* cr, gpointer data)
{
    pthread_mutex_lock(&data_mutex);

    int counts[] = {tcp_count, udp_count, icmp_count, other_count};
    const char* labels[] = {"TCP", "UDP", "ICMP", "Other"};
    double colors[][3] =
        {
            {0.3, 0.6, 0.9}, // TCP: Blue
            {0.3, 0.8, 0.3}, // UDP: Green
            {0.9, 0.8, 0.2}, // ICMP: Yellow
            {0.8, 0.3, 0.3}  // Other: Red
        };

    int total = tcp_count + udp_count + icmp_count + other_count;
    if (total == 0)
        total = 1;

    pthread_mutex_unlock(&data_mutex);

    double angle_start = 0.0;
    double xc = 150, yc = 150, radius = 100;

    for (int i = 0; i < 4; ++i)
    {
        double fraction = (double)counts[i] / total;
        double angle_end = angle_start + fraction * 2 * G_PI;

        cairo_set_source_rgb(cr, colors[i][0], colors[i][1], colors[i][2]);
        cairo_move_to(cr, xc, yc);
        cairo_arc(cr, xc, yc, radius, angle_start, angle_end);
        cairo_close_path(cr);
        cairo_fill(cr);

        angle_start = angle_end;
    }

    double legend_y = 270;
    for (int i = 0; i < 4; i++)
    {
        cairo_set_source_rgb(cr, colors[i][0], colors[i][1], colors[i][2]);
        cairo_rectangle(cr, 10, legend_y + i * 20, 12, 12);
        cairo_fill(cr);

        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_move_to(cr, 30, legend_y + i * 20 + 10);
        cairo_show_text(cr, labels[i]);
    }

    return FALSE;
}

gboolean update_recent_tab()
{
    pthread_mutex_lock(&data_mutex);
    gtk_list_store_clear(store_recent_packets);

    for (int i = 0; i < packet_count; i++)
    {
        GtkTreeIter iter;
        gtk_list_store_append(store_recent_packets, &iter);

        // Format timestamp
        char timebuf[64];
        struct tm* lt = localtime(&recent_packets[i].timestamp);
        strftime(timebuf, sizeof(timebuf), "%H:%M:%S", lt);

        // Copy strings to temporary buffers
        char src_ip_buf[INET_ADDRSTRLEN];
        char dst_ip_buf[INET_ADDRSTRLEN];
        char proto_buf[16];

        strncpy(src_ip_buf, recent_packets[i].src_ip, sizeof(src_ip_buf));
        src_ip_buf[sizeof(src_ip_buf)-1] = '\0';

        strncpy(dst_ip_buf, recent_packets[i].dst_ip, sizeof(dst_ip_buf));
        dst_ip_buf[sizeof(dst_ip_buf)-1] = '\0';

        strncpy(proto_buf, recent_packets[i].protocol, sizeof(proto_buf));
        proto_buf[sizeof(proto_buf)-1] = '\0';

        gtk_list_store_set(store_recent_packets, &iter,
                           0, src_ip_buf,
                           1, dst_ip_buf,
                           2, proto_buf,
                           3, recent_packets[i].size,
                           4, timebuf,
                           -1);
    }

    pthread_mutex_unlock(&data_mutex);
    return FALSE;
}

void show_packet_details(GtkWidget* widget, gpointer data)
{
    int index = GPOINTER_TO_INT(data);
    if (index < 0 || index >= packet_count)
        return;

    Packet* pkt = &recent_packets[index];

    // Copy ctime result into local buffer to avoid newline or other issues
    char timebuf[64];
    strncpy(timebuf, ctime(&pkt->timestamp), sizeof(timebuf));
    timebuf[sizeof(timebuf) - 1] = '\0';

    // Optionally remove trailing newline from ctime string
    size_t len = strlen(timebuf);
    if (len > 0 && timebuf[len - 1] == '\n')
        timebuf[len - 1] = '\0';

    char details[512];
    snprintf(details, sizeof(details),
             "Source IP: %s\n"
             "Destination IP: %s\n"
             "Protocol: %s\n"
             "Size: %d bytes\n"
             "Timestamp: %s",
             pkt->src_ip,
             pkt->dst_ip,
             pkt->protocol,
             pkt->size,
             timebuf);

    GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(nwapp->window),
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_OK,
                                               "%s", details);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

GtkWidget* create_overview_grid()
{
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    overview_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(overview_grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(overview_grid), 12);
    gtk_widget_set_halign(overview_grid, GTK_ALIGN_START);

    GtkWidget* header_metric = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header_metric), "<b>Metric</b>");
    gtk_widget_set_halign(header_metric, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(overview_grid), header_metric, 0, 0, 1, 1);

    GtkWidget* header_value = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header_value), "<b>Value</b>");
    gtk_widget_set_halign(header_value, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(overview_grid), header_value, 1, 0, 1, 1);

#define ADD_ROW(row, label_text, label_var)                                \
    do                                                                     \
    {                                                                      \
        GtkWidget* lbl = gtk_label_new(label_text);                        \
        gtk_widget_set_halign(lbl, GTK_ALIGN_START);                       \
        gtk_grid_attach(GTK_GRID(overview_grid), lbl, 0, row, 1, 1);       \
        label_var = gtk_label_new("0");                                    \
        gtk_widget_set_halign(label_var, GTK_ALIGN_START);                 \
        gtk_grid_attach(GTK_GRID(overview_grid), label_var, 1, row, 1, 1); \
    } while (0)

    ADD_ROW(1, "Total Packets", lbl_total_packets);
    ADD_ROW(2, "Total Bytes", lbl_total_bytes);
    ADD_ROW(3, "TCP", lbl_tcp);
    ADD_ROW(4, "UDP", lbl_udp);
    ADD_ROW(5, "ICMP", lbl_icmp);
    ADD_ROW(6, "Other", lbl_other);

    gtk_box_pack_start(GTK_BOX(vbox), overview_grid, FALSE, FALSE, 0);

    protocol_pie_chart = gtk_drawing_area_new();
    gtk_widget_set_size_request(protocol_pie_chart, 300, 320);
    g_signal_connect(protocol_pie_chart, "draw", G_CALLBACK(draw_protocol_pie), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), protocol_pie_chart, FALSE, FALSE, 0);

    return vbox;
}

gboolean update_overview_gui(gpointer user_data)
{
    pthread_mutex_lock(&data_mutex);

    int total_protocol = tcp_count + udp_count + icmp_count + other_count;
    if (total_protocol == 0)
        total_protocol = 1; // avoid div zero

    char buf[64];

    snprintf(buf, sizeof(buf), "%d", total_packets);
    gtk_label_set_text(GTK_LABEL(lbl_total_packets), buf);

    snprintf(buf, sizeof(buf), "%d", total_bytes);
    gtk_label_set_text(GTK_LABEL(lbl_total_bytes), buf);

    snprintf(buf, sizeof(buf), "%d (%.1f%%)", tcp_count, tcp_count * 100.0 / total_protocol);
    gtk_label_set_text(GTK_LABEL(lbl_tcp), buf);

    snprintf(buf, sizeof(buf), "%d (%.1f%%)", udp_count, udp_count * 100.0 / total_protocol);
    gtk_label_set_text(GTK_LABEL(lbl_udp), buf);

    snprintf(buf, sizeof(buf), "%d (%.1f%%)", icmp_count, icmp_count * 100.0 / total_protocol);
    gtk_label_set_text(GTK_LABEL(lbl_icmp), buf);

    snprintf(buf, sizeof(buf), "%d (%.1f%%)", other_count, other_count * 100.0 / total_protocol);
    gtk_label_set_text(GTK_LABEL(lbl_other), buf);

    pthread_mutex_unlock(&data_mutex);
    return FALSE;
}

// Packet capture thread function
void* capture_packets(void* arg)
{
    thread_running = 1;

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sockfd < 0)
    {
        perror("Socket creation failed");
        thread_running = 0;
        return NULL;
    }

    int opt = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        close(sockfd);
        thread_running = 0;
        return NULL;
    }

    char buffer[65536];
    struct sockaddr saddr;
    socklen_t saddr_size = sizeof(saddr);

    while (capturing)
    {
        ssize_t data_size = recvfrom(sockfd, buffer, sizeof(buffer), 0, &saddr, &saddr_size);
        if (data_size < 0)
        {
            perror("recvfrom");
            continue;
        }

        if (data_size < (ssize_t)sizeof(struct iphdr))
            continue;

        struct iphdr* iph = (struct iphdr *)buffer;
        int ip_header_len = iph->ihl * 4;

        if (data_size < ip_header_len)
            continue;

        char src_ip[INET_ADDRSTRLEN];
        char dst_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &iph->saddr, src_ip, sizeof(src_ip));
        inet_ntop(AF_INET, &iph->daddr, dst_ip, sizeof(dst_ip));

        // Determine protocol string
        const char* proto_str;
        switch (iph->protocol)
        {
            case IPPROTO_TCP:  proto_str = "TCP"; break;
            case IPPROTO_UDP:  proto_str = "UDP"; break;
            case IPPROTO_ICMP: proto_str = "ICMP"; break;
            default:           proto_str = "OTHER"; break;
        }

        pthread_mutex_lock(&data_mutex);
        total_packets++;
        total_bytes += data_size;
        pthread_mutex_unlock(&data_mutex);

        add_ip(src_ip_entries, &src_ip_count, src_ip);
        add_ip(dst_ip_entries, &dst_ip_count, dst_ip);

        switch (iph->protocol)
        {
        case IPPROTO_TCP:
            pthread_mutex_lock(&data_mutex);
            tcp_count++;
            pthread_mutex_unlock(&data_mutex);
            if (data_size >= ip_header_len + (ssize_t)sizeof(struct tcphdr))
            {
                struct tcphdr* tcph = (struct tcphdr *)(buffer + ip_header_len);
                add_port(src_ports, &src_port_count, ntohs(tcph->source));
                add_port(dst_ports, &dst_port_count, ntohs(tcph->dest));
            }
            break;
        case IPPROTO_UDP:
            pthread_mutex_lock(&data_mutex);
            udp_count++;
            pthread_mutex_unlock(&data_mutex);
            if (data_size >= ip_header_len + (ssize_t)sizeof(struct udphdr))
            {
                struct udphdr* udph = (struct udphdr *)(buffer + ip_header_len);
                add_port(src_ports, &src_port_count, ntohs(udph->source));
                add_port(dst_ports, &dst_port_count, ntohs(udph->dest));
            }
            break;
        case IPPROTO_ICMP:
            pthread_mutex_lock(&data_mutex);
            icmp_count++;
            pthread_mutex_unlock(&data_mutex);
            break;
        default:
            pthread_mutex_lock(&data_mutex);
            other_count++;
            pthread_mutex_unlock(&data_mutex);
            break;
        }

        // Protect add_packet if it modifies shared data
        pthread_mutex_lock(&data_mutex);
        add_packet(src_ip, dst_ip, proto_str, data_size);
        pthread_mutex_unlock(&data_mutex);

        // Schedule GUI updates on main thread
        g_idle_add(update_src_ips_gui, NULL);
        g_idle_add(update_dst_ips_gui, NULL);
        g_idle_add(update_src_ports_gui, NULL);
        g_idle_add(update_dst_ports_gui, NULL);
        g_idle_add(update_overview_gui, NULL);
        g_idle_add(update_recent_tab, NULL);
    }

    close(sockfd);
    thread_running = 0;
    return NULL;
}

GtkWidget* create_ip_treeview(GtkListStore* store)
{
    GtkWidget* treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn* col_num = gtk_tree_view_column_new_with_attributes("No.", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col_num);

    GtkTreeViewColumn* col_ip = gtk_tree_view_column_new_with_attributes("IP Address", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col_ip);

    GtkTreeViewColumn* col_host = gtk_tree_view_column_new_with_attributes("Hostname", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col_host);

    GtkTreeViewColumn* col_count = gtk_tree_view_column_new_with_attributes("Packets", renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col_count);

    gtk_widget_set_vexpand(treeview, TRUE);

    return treeview;
}

GtkWidget* create_port_treeview(GtkListStore* store)
{
    GtkWidget* treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn* col_num = gtk_tree_view_column_new_with_attributes("No.", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col_num);

    GtkTreeViewColumn* col_port = gtk_tree_view_column_new_with_attributes("Port", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col_port);

    GtkTreeViewColumn* col_count = gtk_tree_view_column_new_with_attributes("Packets", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col_count);

    gtk_widget_set_vexpand(treeview, TRUE);

    return treeview;
}

void on_recent_packet_activated(GtkTreeView* tree_view,
                               GtkTreePath* path,
                               GtkTreeViewColumn* column,
                               gpointer user_data)
{
    if (!path) return;  // Defensive: ignore if path NULL

    GtkTreeModel* model = gtk_tree_view_get_model(tree_view);
    if (!model) return;

    GtkTreeIter iter;
    if (!gtk_tree_model_get_iter(model, &iter, path))
        return;  // Can't get iter for path, abort

    gchar* src_ip = NULL;
    gchar* dst_ip = NULL;
    gchar* protocol = NULL;
    gchar* time = NULL;
    guint size = 0;

    gtk_tree_model_get(model, &iter,
                       0, &src_ip,
                       1, &dst_ip,
                       2, &protocol,
                       3, &size,
                       4, &time,
                       -1);

    if (!src_ip || !dst_ip || !protocol || !time)
    {
        // Defensive: free any allocated strings if needed, then return
        if (src_ip) g_free(src_ip);
        if (dst_ip) g_free(dst_ip);
        if (protocol) g_free(protocol);
        if (time) g_free(time);
        return;
    }

    char message[512];
    snprintf(message, sizeof(message),
             "Source IP: %s\n"
             "Destination IP: %s\n"
             "Protocol: %s\n"
             "Size: %u bytes\n"
             "Timestamp: %s",
             src_ip, dst_ip, protocol, size, time);

    GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(nwapp->window),
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_OK,
                                               "%s", message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    g_free(src_ip);
    g_free(dst_ip);
    g_free(protocol);
    g_free(time);
}

GtkWidget* create_recent_packets_view()
{
    // Define columns: Source IP, Destination IP, Protocol, Size, Timestamp
    store_recent_packets = gtk_list_store_new(5, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING);

    GtkWidget* treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store_recent_packets));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview), TRUE);

    GtkCellRenderer* renderer = gtk_cell_renderer_text_new();

    GtkTreeViewColumn* col_src = gtk_tree_view_column_new_with_attributes("Source IP", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col_src);

    GtkTreeViewColumn* col_dst = gtk_tree_view_column_new_with_attributes("Destination IP", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col_dst);

    GtkTreeViewColumn* col_proto = gtk_tree_view_column_new_with_attributes("Protocol", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col_proto);

    GtkTreeViewColumn* col_size = gtk_tree_view_column_new_with_attributes("Size (Bytes)", renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col_size);

    GtkTreeViewColumn* col_time = gtk_tree_view_column_new_with_attributes("Time", renderer, "text", 4, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col_time);

    // Allow row-activated signal (double-click)
    g_signal_connect(treeview, "row-activated", G_CALLBACK(on_recent_packet_activated), NULL);

    return treeview;
}

// Main activate function, used as application activate handler
int gtk_netwatch_app_activate(GtkApplication* app, gpointer user_data)
{
    NetWatchApp* nwapp = mp_alloc(mpool_, sizeof(NetWatchApp));
    nwapp = (NetWatchApp *)user_data;

    nwapp->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(nwapp->window), "NetWatch GUI");
    gtk_window_set_default_size(GTK_WINDOW(nwapp->window), 800, 600);

    GtkWidget* notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(nwapp->window), notebook);

    init_liststores();

    // Overview tab with scrolled window
    GtkWidget* overview_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(overview_scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    GtkWidget* overview = create_overview_grid();
    gtk_container_add(GTK_CONTAINER(overview_scroll), overview);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), overview_scroll, gtk_label_new("Overview"));

    // Recent tab
    GtkWidget* recent_packets_view = create_recent_packets_view();
    GtkWidget* recent_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(recent_scroll), recent_packets_view);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), recent_scroll, gtk_label_new("Recent Packets"));

    // Source IP tab
    GtkWidget* src_ip_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(src_ip_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    GtkWidget* src_ip_treeview = create_ip_treeview(store_src_ips);
    gtk_container_add(GTK_CONTAINER(src_ip_scroll), src_ip_treeview);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), src_ip_scroll, gtk_label_new("Source IPs"));

    // Destination IP tab
    GtkWidget* dst_ip_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(dst_ip_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    GtkWidget* dst_ip_treeview = create_ip_treeview(store_dst_ips);
    gtk_container_add(GTK_CONTAINER(dst_ip_scroll), dst_ip_treeview);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), dst_ip_scroll, gtk_label_new("Destination IPs"));

    // Source Ports tab
    GtkWidget* src_port_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(src_port_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    GtkWidget* src_port_treeview = create_port_treeview(store_src_ports);
    gtk_container_add(GTK_CONTAINER(src_port_scroll), src_port_treeview);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), src_port_scroll, gtk_label_new("Source Ports"));

    // Destination Ports tab
    GtkWidget* dst_port_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(dst_port_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    GtkWidget* dst_port_treeview = create_port_treeview(store_dst_ports);
    gtk_container_add(GTK_CONTAINER(dst_port_scroll), dst_port_treeview);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), dst_port_scroll, gtk_label_new("Destination Ports"));

    gtk_widget_show_all(nwapp->window);

    // Start capture thread
    capturing = 1;
    pthread_create(&capture_thread, NULL, capture_packets, NULL);

    return 0;
}
