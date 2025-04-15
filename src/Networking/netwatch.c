#include "netwatch.h"
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

#define BUFFER_SIZE 65536
int running = 1;

int compare_ip_count_desc(const void *a, const void *b)
{
    const struct ip_count *ip_a = (const struct ip_count *)a;
    const struct ip_count *ip_b = (const struct ip_count *)b;

    // Sort in descending order
    if (ip_a->count < ip_b->count)
        return 1;
    if (ip_a->count > ip_b->count)
        return -1;
    return 0;
}

/*
 * Function: timestamp
 * --------------------
 * Returns the current time as a formatted string.
 *
 * returns: The current time as a string.
 */
const char *timestamp()
{
    static char buf[64];
    time_t now = time(NULL);
    strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&now));
    return buf;
}

/*
 * Function: handle_interrupt
 * ---------------------------
 * Handles the Ctrl+C interrupt signal.
 *
 * sig: The signal number.
 */
void handle_interrupt(int sig)
{
    (void)sig; // explicitly ignore the parameter

    // Not exiting the program, just stopping the netwatch
    if (running == 0)
        return;

    running = 0;

    // Additional cleanup to reset terminal settings can go here if needed
    printf("\n[ NETWATCH ] Stopping...\n");
    fflush(stdout);
}

/*
 * Function: resolve_hostname
 * ---------------------------
 * Resolves the hostname from an IP address.
 *
 * ip: The IP address to resolve.
 *
 * returns: The hostname as a string.
 */
const char *resolve_hostname(struct in_addr ip)
{
    struct hostent *host = gethostbyaddr(&ip, sizeof(ip), AF_INET);
    if (host)
        return host->h_name;
    else
        return "Unknown Host";
}

/*
 * Function: netwatch_start
 * -------------------------
 * Starts the network watch on the specified interface.
 *
 * interface: The network interface to monitor.
 * max_hosts: The maximum number of hosts to display.
 */
void netwatch_start(const char *interface, int max_hosts)
{
    int sockfd;
    struct ifreq ifr;

    signal(SIGINT, handle_interrupt); // Ctrl+C handler

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
        return;
    }

    ifr.ifr_flags |= IFF_PROMISC;
    if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0)
    {
        perror("ioctl(SIOCSIFFLAGS)");
        free(buffer);
        return;
    }

    unsigned long total_packets = 0;
    unsigned long total_bytes = 0;
    unsigned long tcp_packets = 0;
    unsigned long udp_packets = 0;
    unsigned long icmp_packets = 0;
    unsigned long other_packets = 0;

    unsigned long last_packets = 0;
    unsigned long last_bytes = 0;

    struct ip_count ip_addresses[256] = {0}; // Store IP counts, increase size if needed
    int ip_count = 0;

    printf("\033[2J\033[H"); // clear screen and move cursor to top

    running = true;

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
            struct in_addr src_ip;
            src_ip.s_addr = iph->saddr; // Assign the IP address properly

            // Count the IP address occurrences
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

            total_packets++;
            total_bytes += data_size;

            // protocol counting
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
            other_packets++;

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        long current_ms = now.tv_sec * 1000 + now.tv_nsec / 1000000;

        static long last_ms = 0;
        if (last_ms == 0)
            last_ms = current_ms; // Initialize

        long elapsed_ms = current_ms - last_ms;
        if (elapsed_ms >= 100) // Refresh every 100ms
        {
            last_ms = current_ms;

            unsigned long pps = (unsigned long)((total_packets - last_packets) * 1000.0 / elapsed_ms);
            unsigned long bps = (unsigned long)((total_bytes - last_bytes) * 1000.0 / elapsed_ms);

            last_packets = total_packets;
            last_bytes = total_bytes;

            // Sort the IP addresses by count in descending order
            for (int i = 0; i < ip_count - 1; i++)
            {
                for (int j = i + 1; j < ip_count; j++)
                {
                    if (ip_addresses[i].count < ip_addresses[j].count)
                    {
                        struct ip_count temp = ip_addresses[i];
                        ip_addresses[i] = ip_addresses[j];
                        ip_addresses[j] = temp;
                    }
                }
            }

            // Output
            printf("\033[H");    // Move cursor to top
            printf("\033[?25l"); // Hide cursor

            __bold;
            __cyan;
            printf("┌─────────────────────────────── NetWatch ──────────────────────────────┐\n");
            printf("│ Interface: %-10s Time (May be inaccurate): %-20s  │\n", interface, timestamp());
            printf("├───────────────────┬───────────────────────────────────────────────────┤\n");
            __green;
            printf("│ Packets captured  │  %-44lu\t│\n", total_packets);
            printf("│ Total bytes (MB)  │  %-44.2f\t│\n", (double)total_bytes / 1000000);
            printf("│ Packets/s         │  %-44.2f\t│\n", (double)pps);
            printf("│ KBytes/s          │  %-44.2f\t│\n", (double)bps / 1000);
            printf("├───────────────────┼───────────────────────────────────────────────────┤\n");
            printf("│ TCP               │  %-5lu \t\t\t\t\t\t│\n", tcp_packets);
            printf("│ UDP               │  %-5lu \t\t\t\t\t\t│\n", udp_packets);
            printf("│ ICMP              │  %-5lu \t\t\t\t\t\t│\n", icmp_packets);
            printf("│ Other             │  %-5lu \t\t\t\t\t\t│\n", other_packets);

            // Display the top 5 IP addresses and their hostnames
            printf("├───────────────────┴───────────────────────────────────────────────────┤\n");
            printf("│ Top %-2d Addresses and Hostnames                                        │\n", max_hosts);
            printf("├───────────────────────┬───────────────────────────────────────────────┤\n");

            for (int i = 0; i < max_hosts && i < ip_count; i++)
            {
                const char *hostname = resolve_hostname(ip_addresses[i].ip);
                printf("│ (%-2d.) %-15s │ %-45s │\n", i + 1, inet_ntoa(ip_addresses[i].ip), hostname);
            }
            printf("│ Total Hosts           │  %-45d│\n", ip_count);

            __yellow;
            printf("├───────────────────────┴───────────────────────────────────────────────┤\n");
            printf("│ Ctrl+C to stop                                                        │\n");
            printf("└───────────────────────────────────────────────────────────────────────┘\n");
            __reset;

            printf("\033[?25h"); // Show cursor

            fflush(stdout);
        }
    }

    // Clean up
    printf("\n[ NETWATCH ] Stopping...\n");
    ifr.ifr_flags &= ~IFF_PROMISC;
    ioctl(sockfd, SIOCSIFFLAGS, &ifr);
    close(sockfd);
    free(buffer);
}
