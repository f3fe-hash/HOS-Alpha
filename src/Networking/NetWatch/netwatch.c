#include "netwatch.h"

#define BUFFER_SIZE 65536

static int running = 1;

int compare_ip_count_desc(const void *a, const void *b)
{
    const struct ip_count *ip_a = (const struct ip_count *)a;
    const struct ip_count *ip_b = (const struct ip_count *)b;
    return (ip_b->count - ip_a->count);
}

const char *timestamp()
{
    static char buf[64];
    time_t now = time(NULL);
    strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&now));
    return buf;
}

void handle_interrupt(int sig)
{
    (void)sig;
    if (!running)
        return;

    running = 0;
    printf("\n[ NETWATCH ] Stopping...\n");
    fflush(stdout);
}

const char *resolve_hostname(struct in_addr ip)
{
    struct hostent *host = gethostbyaddr(&ip, sizeof(ip), AF_INET);
    return (host) ? host->h_name : "Unknown Host";
}

ip_t *scan_hosts(const char *baseip)
{
    ip_t *hosts = malloc(MAX_HOSTS * sizeof(ip_t));
    char *search_ip = malloc(16);

    if (!hosts)
        return NULL;

    for (int i = 0; i < MAX_HOSTS; i++)
    {
        // Check host
        snprintf(search_ip, sizeof(ip_t) / sizeof(char), "%s.%d", baseip, i);
        if (!ping(inet_addr(search_ip), 64, 1, 10000))
            hosts[i] = (ip_t)search_ip;
    }

    return hosts;
}

void netwatch_start(const char *interface, int max_hosts)
{
    int sockfd;
    struct ifreq ifr;

    signal(SIGINT, handle_interrupt);

    unsigned char *buffer = malloc(BUFFER_SIZE);
    if (!buffer)
        return;

    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0)
    {
        perror("socket");
        free(buffer);
        return;
    }

    strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);
    if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
    {
        perror("ioctl(SIOCGIFFLAGS)");
        free(buffer);
        close(sockfd);
        return;
    }

    ifr.ifr_flags |= IFF_PROMISC;
    if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0)
    {
        perror("ioctl(SIOCSIFFLAGS)");
        free(buffer);
        close(sockfd);
        return;
    }

    unsigned long total_packets = 0, total_bytes = 0;
    unsigned long tcp_packets = 0, udp_packets = 0, icmp_packets = 0, other_packets = 0;
    unsigned long last_packets = 0, last_bytes = 0;
    time_t last_refresh = time(NULL);

    struct ip_count ip_addresses[256] = {0};
    int ip_count = 0;

    printf("\033[2J\033[H"); // clear screen
    running = 1;

    while (running)
    {
        ssize_t data_size = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
        if (data_size < 0)
        {
            if (errno == EINTR)
                break;
            perror("recvfrom");
            break;
        }

        total_packets++;
        total_bytes += data_size;

        struct ethhdr *eth = (struct ethhdr *)buffer;
        if (ntohs(eth->h_proto) == ETH_P_IP)
        {
            struct iphdr *iph = (struct iphdr *)(buffer + sizeof(struct ethhdr));
            struct in_addr src_ip = {.s_addr = iph->saddr};

            int found = 0;
            for (int i = 0; i < ip_count; i++)
            {
                if (memcmp(&ip_addresses[i].ip, &src_ip, sizeof(struct in_addr)) == 0)
                {
                    ip_addresses[i].count++;
                    found = 1;
                    break;
                }
            }
            if (!found && ip_count < 256)
            {
                ip_addresses[ip_count].ip = src_ip;
                ip_addresses[ip_count].count = 1;
                ip_count++;
            }

            switch (iph->protocol)
            {
            case IPPROTO_TCP:
                tcp_packets++;
                break;
            case IPPROTO_UDP:
                udp_packets++;
                break;
            case IPPROTO_ICMP:
                icmp_packets++;
                break;
            default:
                other_packets++;
                break;
            }
        }
        else
        {
            other_packets++;
        }

        if (time(NULL) - last_refresh >= 1)
        {
            last_refresh = time(NULL);

            unsigned long pps = total_packets - last_packets;
            unsigned long bps = total_bytes - last_bytes;
            last_packets = total_packets;
            last_bytes = total_bytes;

            ip_t *hosts = scan_hosts("192.168.1");
            int total_hosts = sizeof(hosts) / sizeof(ip_t);

            qsort(ip_addresses, ip_count, sizeof(struct ip_count), compare_ip_count_desc);

            __clear;
            __bold;
            __cyan;
            printf("┌─────────────────────────────── NetWatch ──────────────────────────────┐\n");
            printf("│ Interface: %-10s Time: %-40s  │\n", interface, timestamp());
            printf("├───────────────────┬───────────────────────────────────────────────────┤\n");
            __green;
            printf("│ Packets captured  │  %-44lu\t│\n",  (unsigned long)total_packets);
            printf("│ Total MBytes      │  %-42.2f\t│\n", (float)        total_bytes / 1000000.0);
            printf("│ Packets/s         │  %-44lu\t│\n",  (unsigned long)pps);
            printf("│ KBytes/s          │  %-41.2f\t│\n", (float)        bps / 1000.0);
            printf("├───────────────────┼───────────────────────────────────────────────────┤\n");
            printf("│ TCP               │  %-5lu \t\t\t\t\t\t│\n", (unsigned long)tcp_packets);
            printf("│ UDP               │  %-5lu \t\t\t\t\t\t│\n", (unsigned long)udp_packets);
            printf("│ ICMP              │  %-5lu \t\t\t\t\t\t│\n", (unsigned long)icmp_packets);
            printf("│ Other             │  %-5lu \t\t\t\t\t\t│\n", (unsigned long)other_packets);
            printf("├───────────────────┴────────────────┬──────────────────────────────────┤\n");
            printf("│ Top %-2d Addresses and Hostnames     │   %-2d Total Host(s)               │\n", max_hosts, ip_count);
            printf("├───────────────────────┬────────────┴──────────────────────────────────┤\n");

            for (int i = 0; i < max_hosts && i < ip_count; i++)
            {
                const char *hostname = resolve_hostname(ip_addresses[i].ip);
                printf("│ (%-2d.) %-15s │ %-45s │\n", i + 1, inet_ntoa(ip_addresses[i].ip), hostname);
            }

            __yellow;
            printf("├───────────────────────┴────────┬───────────────────────────────────────┤\n");
            printf("│ Hosts found on current network │ Total Hosts: %20lu                 │\n", (unsigned long)total_hosts);
            printf("├────────────────────────────────┴──────────────────────────────────────┤\n");

            for (int i = 0; i < total_hosts; i++)
                printf("│ %s \t\t\t\t\t\t\t\t│\n", hosts[i]);

            printf("├───────────────────────────────────────────────────────────────────────┤\n");
            printf("│                           Ctrl+C to stop                              │\n");
            printf("└───────────────────────────────────────────────────────────────────────┘\n");
            __reset;

            fflush(stdout);
        }
    }

    printf("\n[ NETWATCH ] Stopping...\n");
    ifr.ifr_flags &= ~IFF_PROMISC;
    ioctl(sockfd, SIOCSIFFLAGS, &ifr);
    close(sockfd);
    free(buffer);
}
