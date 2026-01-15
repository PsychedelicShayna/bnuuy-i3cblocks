#include <arpa/inet.h>
#include <bits/wordsize.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <features.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <locale.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <net/route.h>
#include <netdb.h>
#include <netinet/in.h>
#include <rte_common.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <unistdio.h>
#include <unitypes.h>
#include <wchar.h>

#include "color/color.h"
#include "i3bar.h"
#include "pango.h"

#define PUBLIC_IP_API "https://myip.wtf/"
#define MONITOR_IFACE "enp7s0"

typedef struct {
    uint64_t tx_bytes;
    uint64_t rx_bytes;
} rx_tx_stats_t;

typedef struct {
    rx_tx_stats_t s1;
    rx_tx_stats_t s2;
    rx_tx_stats_t rate;
} net_io_rate_t;

// samples the network transmitted and received rate
net_io_rate_t sample_net_io_rate(void)
{

    net_io_rate_t nior;
    const char *path_rx =
                 "/sys/class/net/" MONITOR_IFACE "/statistics/rx_bytes",

               *path_tx =
                 "/sys/class/net/" MONITOR_IFACE "/statistics/tx_bytes";

    FILE *fd_rx = fopen(path_rx, "r"), *fd_tx = fopen(path_tx, "r");

    assert(fd_rx != NULL);
    assert(fd_tx != NULL);

    fscanf(fd_rx, "%lu", &nior.s1.rx_bytes);
    fscanf(fd_tx, "%lu", &nior.s1.tx_bytes);

    fseek(fd_rx, 0, 0);
    fseek(fd_tx, 0, 0);

    usleep(1000000);

    fscanf(fd_rx, "%lu", &nior.s2.rx_bytes);
    fscanf(fd_tx, "%lu", &nior.s2.tx_bytes);

    fclose(fd_rx);
    fclose(fd_tx);

    nior.rate.rx_bytes = nior.s2.rx_bytes - nior.s1.rx_bytes;
    nior.rate.tx_bytes = nior.s2.tx_bytes - nior.s1.tx_bytes;

    return nior;
}

typedef struct {
    char     ipv4[NI_MAXHOST];
    char     ipv6[NI_MAXHOST];
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_rate;
    uint64_t tx_rate;
} ifstats_t;

void print_route(void)
{
}

void get_iface_stats(const char*  iface,
                     ifstats_t*   out,
                     __useconds_t samplesleep)
{
    memset(out->ipv4, 0, NI_MAXHOST);
    memset(out->ipv6, 0, NI_MAXHOST);

    *out =
      (ifstats_t) { .rx_rate = 0, .tx_rate = 0, .rx_bytes = 0, .tx_bytes = 0 };

    struct ifaddrs* ifap;

    if(getifaddrs(&ifap) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    for(struct ifaddrs* ifa = ifap; ifa != NULL; ifa = ifa->ifa_next) {
        const char* ifname = ifa->ifa_name;

        if(strcmp(ifname, iface)) {
            continue;
        }

        int family = ifa->ifa_addr->sa_family;

        if(family == AF_INET || family == AF_INET6) {
            int status =
              getnameinfo(ifa->ifa_addr,
                          family == AF_INET ? sizeof(struct sockaddr_in)
                                            : sizeof(struct sockaddr_in6),
                          family == AF_INET ? out->ipv4 : out->ipv6,
                          NI_MAXHOST,
                          NULL,
                          0,
                          NI_NUMERICHOST);

            if(status != 0) {
                fprintf(
                  stderr, "getnameinfo() failed: %s\n", gai_strerror(status));
                exit(EXIT_FAILURE);
            }

        } else if(family == AF_PACKET && ifa->ifa_data != NULL) {
            struct rtnl_link_stats* stats = ifa->ifa_data;

            out->rx_bytes = stats->rx_bytes;
            out->tx_bytes = stats->tx_bytes;
        }
    }

    freeifaddrs(ifap);

    usleep(samplesleep);

    if(getifaddrs(&ifap) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    for(struct ifaddrs* ifa = ifap; ifa != NULL; ifa = ifa->ifa_next) {
        if(strcmp(ifa->ifa_name, iface)) {
            continue;
        }

        if(ifa->ifa_addr->sa_family == AF_PACKET && ifa->ifa_data != NULL) {
            struct rtnl_link_stats* stats = ifa->ifa_data;

            out->rx_rate  = stats->rx_bytes - out->rx_bytes;
            out->rx_bytes = stats->rx_bytes;

            out->tx_rate  = stats->tx_bytes - out->tx_bytes;
            out->tx_bytes = stats->tx_bytes;
        }
    }

    freeifaddrs(ifap);
}

void _curl_recv_ip(const void* data, ulong size, ulong n, struct in_addr* out)
{
    char buffer[NI_MAXHOST];
    memset(buffer, 0, NI_MAXHOST);
    snprintf(buffer, size * n, "%s", (char*)data);
    out->s_addr = inet_addr(buffer);
}

void get_public_ip(char* out, size_t szout)
{
    struct in_addr ipv4;
    CURL*          curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, PUBLIC_IP_API);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 4L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.68.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _curl_recv_ip);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ipv4);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    char* ipv4_str = inet_ntoa(ipv4);
    strncpy(out, ipv4_str, szout);
    // memcpy(out, ipv4_str, strlen(ipv4_str));
}

typedef enum {
    NO_PUBLIC_IP,      // internet is fine but no public ip from myip.wtf
    NO_PRIVATE_IP,     // ifstats didn't return private IP.
    NO_PING_LOCALHOST, // 127.0.0.1
    NO_PING_ROUTER,    // 192.168.1.1
    NO_PING_INTERNET,  // 8.8.8.8

} NETWORK_STATE;

void output(void)
{
    setlocale(LC_ALL, "");
    // Only now and then get the public IP.

    // We can schedule it without threads using
    // a simple counter. For example, every 60 iterations.

    uint64_t ticks = 0;

    panspan span;
    panspan_init(&span);
    span.foreground = rgbx(BLUE);

    wchar_t* full_text = malloc(sizeof(wchar_t) * 4096);
    memset(full_text, 0, 4096);

    i3bar_block_t block;
    i3bar_block_init(&block);
    block.full_text = full_text;
    block.markup    = "pango";

    wchar_t buffer[4096];
    memset(buffer, 0, 4096);

    ifstats_t ifstats;
    block.separator = false;

    char pub[NI_MAXHOST];
    memset(pub, 0, NI_MAXHOST);

    do {
        get_iface_stats(MONITOR_IFACE, &ifstats, 1000000);

        // we need public IP, LAN IP, NET IO, and possibly online status.

        if(ticks % 30 == 0) {
            // char new_public[16];
            // memset(new_public, 0, 16);
            get_public_ip(pub, NI_MAXHOST);

            if(strlen(pub) < 1) {
                memcpy(pub, "N/A\0", 4);
            }
        }

        swprintf(buffer,
                 4096,
                 L" %5.3lfM↓ %5.3lfM↑ %s 󰈀 %s   ",
                 ((double)ifstats.rx_rate / 1024.0 / 1024.0),
                 ((double)ifstats.tx_rate / 1024.0 / 1024.0),
                 ifstats.ipv4,
                 pub);

        pango_format(full_text, 4096, buffer, PAT_NULL, &span);
        i3bar_block_output(&block);
        full_text[0] = 0x00;
    } while(++ticks);
}

int main(void)
{
    output();
    // Network IO Rate 
    // Ethernet LAN IP 󰈀
    // Public IPv4 IP  

    return 0;
    while(1) {
        // char buf[NI_MAXHOST];
        // memset(buf, 0, size_t n)
        //
        //   in_addr_t pub = get_public_ip();
        //
        // ifstats_t stats;
        // get_iface_stats(MONITOR_IFACE, &stats, 1000000);
        //
        // // echo "${RB}K↓ ${TB}K↑"
        // printf("%5dK↓  %5dK↑\n", stats.tx_rate / 1024, stats.rx_rate / 1024);
        // printf("----------------\n");
        //
        // struct in_addr pub2;
        // pub2.s_addr = pub;
        //
        // // char* using_pton = inet_net_ntop(int af, const char *cp, void
        // *buf,
        // // size_t len)n
        // char buf[12];
        // memset(buf, 0, 12);
        // char* using_ntop = inet_ntoa(pub2);
        // printf("Pub2: %s\n", using_ntop);
        //
        // fflush_unlocked(stdout);
        //
        // net_io_rate_t sample = sample_net_io_rate();
        // // echo "${RB}K↓ ${TB}K↑"
        // printf("%5dK↓  %5dK↑\n",
        //        sample.rate.rx_bytes / 1024,
        //        sample.rate.tx_bytes / 1024);
    }

    // sample_net_io_rate();
    // in_addr_t ipv4  = get_public_ip();
    // uint8_t*  ipv4b = (uint8_t*)&ipv4;
    //
    // printf("%d.%d.%d.%d\n", ipv4b[0], ipv4b[1], ipv4b[2], ipv4b[3]);

    // in_addr_t       a;
    // struct addrinfo b;
    // phys_addr_t     c;
    // struct in6_addr d;
}
