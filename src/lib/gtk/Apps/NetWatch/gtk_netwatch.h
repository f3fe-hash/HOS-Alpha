#ifndef __GTK_NETWATCH_H__
#define __GTK_NETWATCH_H__

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <pthread.h>

#include "../app_base.h"

#define MAX_IPS 1024
#define MAX_PORTS 512
#define MAX_RECENT_PACKETS 100

typedef struct
{
    uint16_t port;
    int count;
} PortEntry;

typedef struct
{
    char ip[INET_ADDRSTRLEN];
    char hostname[256];
    int packet_count;
    time_t last_seen;
} IPEntry;

typedef struct
{
    char src_ip[16];
    char dst_ip[16];
    char protocol[8];
    int size;
    time_t timestamp;
} Packet;

typedef struct
{
    GtkWidget *window;
    GtkWidget *treeview;
    GtkListStore *store;
    const char *interface;
    int max_hosts;
} NetWatchApp;

// Global state
extern IPEntry src_ip_entries[MAX_IPS];
extern int src_ip_count;

extern IPEntry dst_ip_entries[MAX_IPS];
extern int dst_ip_count;

extern PortEntry src_ports[MAX_PORTS];
extern int src_port_count;

extern PortEntry dst_ports[MAX_PORTS];
extern int dst_port_count;

extern NetWatchApp *nwapp;

extern GtkListStore *store_src_ips;
extern GtkListStore *store_dst_ips;
extern GtkListStore *store_src_ports;
extern GtkListStore *store_dst_ports;
extern GtkListStore *store_recent_packets;

extern GtkWidget *overview_grid;
extern GtkWidget *lbl_total_packets;
extern GtkWidget *lbl_total_bytes;
extern GtkWidget *lbl_tcp;
extern GtkWidget *lbl_udp;
extern GtkWidget *lbl_icmp;
extern GtkWidget *lbl_other;
extern GtkWidget *lbl_overview;

extern GtkWidget *protocol_pie_chart;

extern GtkWidget *packet_list;

extern pthread_t capture_thread;
extern volatile int capturing;
extern volatile int thread_running;

extern int total_packets;
extern int total_bytes;
extern int tcp_count, udp_count, icmp_count, other_count;

extern Packet recent_packets[MAX_RECENT_PACKETS];
extern int packet_count;

extern pthread_mutex_t data_mutex;

void add_ip(IPEntry *entries, int *count, const char *ip);
void add_port(PortEntry *ports, int *count, uint16_t port);

int compare_ip_entries(const void *a, const void *b);
int compare_port_entries(const void *a, const void *b);

gboolean update_src_ips_gui(gpointer user_data);
gboolean update_dst_ips_gui(gpointer user_data);
gboolean update_src_ports_gui(gpointer user_data);
gboolean update_dst_ports_gui(gpointer user_data);
gboolean update_recent_tab();
gboolean draw_protocol_pie(GtkWidget *widget, cairo_t *cr, gpointer data);

void init_liststores();
void gtk_netwatch_stop();

void show_packet_details(GtkWidget *widget, gpointer data);
void add_packet(const char *src_ip, const char *dst_ip, const char *protocol, int size);
void on_recent_packet_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);

GtkWidget *create_ip_treeview(GtkListStore *store);
GtkWidget *create_port_treeview(GtkListStore *store);
GtkWidget *create_recent_packets_view();

int gtk_netwatch_app_activate(GtkApplication *app, gpointer user_data);

#endif
