#ifndef UTILS_H
#define UTILS_H

#include <netinet/in.h>

long timeval_diff_ms(struct timeval *start, struct timeval *end);
int resolve_hostname(const char *host, struct sockaddr_in *dest_addr);
int create_udp_socket(void);
int create_icmp_socket(void);
int set_socket_timeout(int sockfd, int seconds, int microseconds);
int set_socket_ttl(int sockfd, int ttl);
ssize_t send_udp_packet(int sockfd, struct sockaddr_in *dest_addr);
ssize_t receive_icmp_packet(int sockfd, char *buf, size_t bufsize, struct sockaddr_in *from_addr, socklen_t *addrlen);

#endif
