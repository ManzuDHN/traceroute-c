#define _GNU_SOURCE
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>

#include "utils.h"

// calcolo RTT
long timeval_diff_ms(struct timeval *start, struct timeval *end) {
    return (end->tv_sec - start->tv_sec) * 1000 + (end->tv_usec - start->tv_usec) / 1000;
}

// conversione hostname o ip e metterlo in sockaddr_in per usare la struttura 
int resolve_hostname(const char *hostname, struct sockaddr_in *dest_addr) {
    
    // azzera struttura e imposta uso ipv4
    memset(dest_addr, 0, sizeof(struct sockaddr_in));
    dest_addr->sin_family = AF_INET;

    // prova a convertire da stringa a ip binario
    if (inet_pton(AF_INET, hostname, &(dest_addr->sin_addr)) == 1) {
        return 0;
    }

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints)); // azzero struttura
    hints.ai_family = AF_INET; // voglio solo risultati ipv4
    hints.ai_socktype = SOCK_STREAM; // per filtrare risultati

    int err = getaddrinfo(hostname, NULL, &hints, &res); // risolve nome in ip
    if (err != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(err));
        return -1;
    }

    struct sockaddr *generic_addr = res->ai_addr;
    if (generic_addr->sa_family == AF_INET) {
        struct sockaddr_in *ipv4_addr = (struct sockaddr_in *)generic_addr;
        dest_addr->sin_addr = ipv4_addr->sin_addr;
    } else {
        freeaddrinfo(res);
        fprintf(stderr, "Errore: l'indirizzo non è IPv4\n");
        return -1;
    }

    freeaddrinfo(res);
    return 0;
}

int create_udp_socket(void) {
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // ipv4, datagramma, udp, 
    if (sock < 0) perror("e: create_udp_socket");
    return sock;
}

int create_icmp_socket(void) {
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); // ipv4, no intestazioni e possibile ricevere icmp, icmp
    if (sock < 0) perror("e: create_icmp_socket");
    return sock;
}

int set_socket_timeout(int sockfd, int seconds, int microseconds) { // imposta timeout di ricezione, dopo errore
    struct timeval timeout = {seconds, microseconds};
    return setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

int set_socket_ttl(int sockfd, int ttl) {
    return setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
}

ssize_t send_udp_packet(int sockfd, struct sockaddr_in *dest_addr) { 
    return sendto(sockfd, NULL, 0, 0, (struct sockaddr *)dest_addr, sizeof(*dest_addr));
}

ssize_t receive_icmp_packet(int sockfd, char *buf, size_t bufsize, struct sockaddr_in *from_addr, socklen_t *addrlen) {
    return recvfrom(sockfd, buf, bufsize, 0, (struct sockaddr *)from_addr, addrlen);
} // salva pacchetto nel buf, restituisce ip mittente 
