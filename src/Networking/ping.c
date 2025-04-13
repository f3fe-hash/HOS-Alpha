#include "ping.h"
#include <netinet/ip.h>      // struct iphdr
#include <netinet/ip_icmp.h> // struct icmphdr
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>

/*
 * Function: calculate_checksum
 * ----------------------------
 * Calculates the checksum for an ICMP packet.
 */
unsigned short calculate_checksum(unsigned short *buffer, int length)
{
    unsigned long sum = 0;

    while (length > 1)
    {
        sum += *buffer++;
        length -= 2;
    }

    if (length == 1)
        sum += *(unsigned char *)buffer;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

/*
 * Function: ping
 * ---------------
 * Sends an ICMP echo request to the specified target.
 *
 * ip_address: The target IP address (in network byte order).
 * timeout: The timeout for the ping in microseconds (1/10^6).
 *
 * returns: 0 on success, non-zero on failure.
 */
int ping(int ip_address, int packet_size, int packets, int timeout)
{
    struct sockaddr_in dest_addr;
    struct timeval tv;

    if (packet_size < (int)sizeof(struct icmphdr))
    {
        fprintf(stderr, "Packet size must be at least %lu bytes\n", sizeof(struct icmphdr));
        return 1;
    }

    // Set timeout
    tv.tv_sec = timeout / 1000000;
    tv.tv_usec = timeout % 1000000;

    // Create raw socket
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0)
    {
        __fail;
        perror("Socket creation failed. Are you running as root?");
        return 1;
    }

    // Set socket timeout
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv)) < 0)
    {
        __fail;
        perror("Failed to set socket timeout");
        close(sockfd);
        return 1;
    }

    // Destination
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = ip_address;

    // Allocate memory for ICMP packet
    char *packet = malloc(packet_size);
    if (!packet)
    {
        perror("Memory allocation failed");
        close(sockfd);
        return 1;
    }

    struct icmphdr *icmp_hdr = (struct icmphdr *)packet;
    char *data = packet + sizeof(struct icmphdr);
    memset(data, 'A', packet_size - sizeof(struct icmphdr));

    for (int i = 0; i < packets; i++)
    {
        // Prepare ICMP packet
        memset(packet, 0, packet_size);
        icmp_hdr->type = ICMP_ECHO;
        icmp_hdr->code = 0;
        icmp_hdr->un.echo.id = getpid();
        icmp_hdr->un.echo.sequence = i + 1;
        memcpy(data, "PINGDATA", packet_size - sizeof(struct icmphdr));
        icmp_hdr->checksum = 0;
        icmp_hdr->checksum = calculate_checksum((unsigned short *)packet, packet_size);

        // Send packet
        if (sendto(sockfd, packet, packet_size, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
        {
            __fail;
            perror("Error sending ping packet");
            continue;
        }

        // Receive response
        char recv_buffer[1024];
        socklen_t addr_len = sizeof(dest_addr);
        ssize_t received_bytes = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0,
                                          (struct sockaddr *)&dest_addr, &addr_len);
        if (received_bytes < 0)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                __fail;
                printf("Request timed out\n");
            }
            else
            {
                __fail;
                perror("Error receiving ping response");
            }
            continue;
        }

        // Process response
        struct iphdr *ip_header = (struct iphdr *)recv_buffer;
        struct icmphdr *icmp_header = (struct icmphdr *)(recv_buffer + (ip_header->ihl << 2));

        if (icmp_header->type == ICMP_ECHOREPLY)
        {
            char src_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &ip_header->saddr, src_ip, sizeof(src_ip));

            __ok;
            printf("Reply from %s: bytes=%zd time=%.3f ms TTL=%d\n",
                   src_ip,
                   received_bytes - (ip_header->ihl << 2),
                   (double)tv.tv_sec * 1000.0 + (double)tv.tv_usec / 1000.0,
                   ip_header->ttl);
        }
        else
        {
            __warn;
            printf("Received ICMP type %d, code %d\n", icmp_header->type, icmp_header->code);
        }
    }

    free(packet);
    close(sockfd);
    return 0;
}