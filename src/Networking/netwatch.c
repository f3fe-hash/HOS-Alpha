#include "netwatch.h"
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

#define BUFFER_SIZE 65536
static int running = 1;

static const char *timestamp()
{
    static char buf[64];
    time_t now = time(NULL);
    strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&now));
    return buf;
}

// Handle the interruption by restoring the terminal to a clean state
void handle_interrupt(int sig)
{
    (void)sig;  // explicitly ignore the parameter
    running = 0;

    // Additional cleanup to reset terminal settings can go here if needed
    printf("\n[ NETWATCH ] Stopping...\n");
    fflush(stdout);
}

// Function to resolve IP address to hostname
const char *resolve_hostname(struct in_addr ip)
{
    struct hostent *host = gethostbyaddr(&ip, sizeof(ip), AF_INET);
    if (host)
        return host->h_name;
    else
        return "Unknown Host";
}

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

    time_t last_refresh = time(NULL);

    // Data structure to hold IP address counts
    struct ip_count
    {
        struct in_addr ip;
        unsigned long count;
    };

    struct ip_count ip_addresses[256]; // Store IP counts, increase size if needed
    int ip_count = 0;

    printf("\033[2J\033[H"); // clear screen and move cursor to top

    while (running)
    {
        ssize_t data_size = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
        if (data_size < 0)
        {
            if (errno == EINTR) break;
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
            src_ip.s_addr = iph->saddr;  // Assign the IP address properly

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

            switch (iph->protocol)
            {
                case IPPROTO_TCP:  tcp_packets++; break;
                case IPPROTO_UDP:  udp_packets++; break;
                case IPPROTO_ICMP: icmp_packets++; break;
                default: other_packets++; break;
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
            printf("\033[H"); // move cursor to top

            __bold; __cyan;
            printf("┌─────────────────────────────── NetWatch ──────────────────────────────┐\n");
            printf("│ Interface: %-10s Time: %-40s  │\n", interface, timestamp());
            printf("├───────────────────┼───────────────────────────────────────────────────┤\n");
            __green;
            printf("│ Packets captured  │  %-44lu\t│\n", total_packets);
            printf("│ Total bytes       │  %-44lu\t│\n", total_bytes);
            printf("│ Packets/s         │  %-44lu\t│\n", pps);
            printf("│ Bytes/s           │  %-44lu\t│\n", bps);
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
