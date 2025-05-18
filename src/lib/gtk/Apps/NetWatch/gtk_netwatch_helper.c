#include "gtk_netwatch.h"

// Helper functions
void add_ip(IPEntry *entries, int *count, const char *ip)
{
    pthread_mutex_lock(&data_mutex);
    for (int i = 0; i < *count; i++)
    {
        if (strcmp(entries[i].ip, ip) == 0)
        {
            entries[i].packet_count++;
            entries[i].last_seen = time(NULL);
            pthread_mutex_unlock(&data_mutex);
            return;
        }
    }
    if (*count >= MAX_IPS)
    {
        pthread_mutex_unlock(&data_mutex);
        return;
    }

    strncpy(entries[*count].ip, ip, INET_ADDRSTRLEN);
    entries[*count].ip[INET_ADDRSTRLEN - 1] = '\0';

    struct in_addr addr;
    inet_pton(AF_INET, ip, &addr);
    struct hostent *he = gethostbyaddr(&addr, sizeof(addr), AF_INET);
    if (he)
        strncpy(entries[*count].hostname, he->h_name, sizeof(entries[*count].hostname));
    else
        strcpy(entries[*count].hostname, "Unknown");

    entries[*count].hostname[sizeof(entries[*count].hostname) - 1] = '\0';

    entries[*count].packet_count = 1;
    entries[*count].last_seen = time(NULL);
    (*count)++;
    pthread_mutex_unlock(&data_mutex);
}

void add_port(PortEntry *ports, int *count, uint16_t port)
{
    pthread_mutex_lock(&data_mutex);
    for (int i = 0; i < *count; i++)
    {
        if (ports[i].port == port)
        {
            ports[i].count++;
            pthread_mutex_unlock(&data_mutex);
            return;
        }
    }
    if (*count >= MAX_PORTS)
    {
        pthread_mutex_unlock(&data_mutex);
        return;
    }
    ports[*count].port = port;
    ports[*count].count = 1;
    (*count)++;
    pthread_mutex_unlock(&data_mutex);
}

// Sorting helpers
int compare_ip_entries(const void *a, const void *b)
{
    const IPEntry *ia = (const IPEntry *)a;
    const IPEntry *ib = (const IPEntry *)b;
    return ib->packet_count - ia->packet_count;
}

int compare_port_entries(const void *a, const void *b)
{
    const PortEntry *pa = (const PortEntry *)a;
    const PortEntry *pb = (const PortEntry *)b;
    return pb->count - pa->count;
}

// Update GUI functions - must access data with lock
gboolean update_src_ips_gui(gpointer user_data)
{
    pthread_mutex_lock(&data_mutex);
    gtk_list_store_clear(store_src_ips);
    qsort(src_ip_entries, src_ip_count, sizeof(IPEntry), compare_ip_entries);

    for (int i = 0; i < src_ip_count; i++)
    {
        GtkTreeIter iter;
        gtk_list_store_append(store_src_ips, &iter);
        gtk_list_store_set(store_src_ips, &iter,
                           0, i + 1,
                           1, src_ip_entries[i].ip,
                           2, src_ip_entries[i].hostname,
                           3, src_ip_entries[i].packet_count,
                           -1);
    }
    pthread_mutex_unlock(&data_mutex);
    return FALSE;
}

gboolean update_dst_ips_gui(gpointer user_data)
{
    pthread_mutex_lock(&data_mutex);
    gtk_list_store_clear(store_dst_ips);
    qsort(dst_ip_entries, dst_ip_count, sizeof(IPEntry), compare_ip_entries);

    for (int i = 0; i < dst_ip_count; i++)
    {
        GtkTreeIter iter;
        gtk_list_store_append(store_dst_ips, &iter);
        gtk_list_store_set(store_dst_ips, &iter,
                           0, i + 1,
                           1, dst_ip_entries[i].ip,
                           2, dst_ip_entries[i].hostname,
                           3, dst_ip_entries[i].packet_count,
                           -1);
    }
    pthread_mutex_unlock(&data_mutex);
    return FALSE;
}

gboolean update_src_ports_gui(gpointer user_data)
{
    pthread_mutex_lock(&data_mutex);
    gtk_list_store_clear(store_src_ports);
    qsort(src_ports, src_port_count, sizeof(PortEntry), compare_port_entries);

    for (int i = 0; i < src_port_count; i++)
    {
        GtkTreeIter iter;
        gtk_list_store_append(store_src_ports, &iter);
        gtk_list_store_set(store_src_ports, &iter,
                           0, i + 1,
                           1, src_ports[i].port,
                           2, src_ports[i].count,
                           -1);
    }
    pthread_mutex_unlock(&data_mutex);
    return FALSE;
}

gboolean update_dst_ports_gui(gpointer user_data)
{
    pthread_mutex_lock(&data_mutex);
    gtk_list_store_clear(store_dst_ports);
    qsort(dst_ports, dst_port_count, sizeof(PortEntry), compare_port_entries);

    for (int i = 0; i < dst_port_count; i++)
    {
        GtkTreeIter iter;
        gtk_list_store_append(store_dst_ports, &iter);
        gtk_list_store_set(store_dst_ports, &iter,
                           0, i + 1,
                           1, dst_ports[i].port,
                           2, dst_ports[i].count,
                           -1);
    }
    pthread_mutex_unlock(&data_mutex);
    return FALSE;
}

void add_packet(const char *src_ip, const char *dst_ip, const char *protocol, int size)
{
    if (packet_count >= MAX_RECENT_PACKETS)
    {
        memmove(&recent_packets[0], &recent_packets[1], sizeof(Packet) * (MAX_RECENT_PACKETS - 1));
        packet_count = MAX_RECENT_PACKETS - 1;
    }

    strncpy(recent_packets[packet_count].src_ip, src_ip, sizeof(recent_packets[packet_count].src_ip));
    strncpy(recent_packets[packet_count].dst_ip, dst_ip, sizeof(recent_packets[packet_count].dst_ip));
    strncpy(recent_packets[packet_count].protocol, protocol, sizeof(recent_packets[packet_count].protocol));
    recent_packets[packet_count].size = size;
    recent_packets[packet_count].timestamp = time(NULL);
    packet_count++;

    g_idle_add((GSourceFunc)update_recent_tab, NULL);
}

void init_liststores()
{
    store_recent_packets = gtk_list_store_new(5, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING);
    store_src_ips = gtk_list_store_new(4, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
    store_dst_ips = gtk_list_store_new(4, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
    store_src_ports = gtk_list_store_new(3, G_TYPE_INT, G_TYPE_UINT, G_TYPE_INT);
    store_dst_ports = gtk_list_store_new(3, G_TYPE_INT, G_TYPE_UINT, G_TYPE_INT);
}

// Function to clean up on app exit
void gtk_netwatch_stop()
{
    capturing = 0;
    if (thread_running)
        pthread_join(capture_thread, NULL);
}