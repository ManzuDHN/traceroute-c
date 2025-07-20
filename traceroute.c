#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>

#include "utils.h"

#define MAX_HOPS 30
#define BASE_PORT 33434

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <hostname o IP>\n", argv[0]);
        return -1;
    }

    struct sockaddr_in dest_addr; // IP del destinatario
    if (resolve_hostname(argv[1], &dest_addr) != 0) {
        fprintf(stderr, "Errore nella risoluzione dell'host %s\n", argv[1]);
        return 1;
    }

    // conversione IP contenuto in dest_addr.sin_addr in stringa leggibile da salvare in ip_str
    char ip_str[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &(dest_addr.sin_addr), ip_str, sizeof(ip_str)) == NULL) {
        perror("inet_ntop");
        return 1;
    }

    // creazione socket udp
    int socket_UDP = create_udp_socket();
    if (socket_UDP < 0) return -1;

    // creazione socket icmp
    int socket_ICMP = create_icmp_socket();
    if (socket_ICMP < 0) {
        close(socket_UDP);
        return -1;
    }

    // attende 2 secondi
    if (set_socket_timeout(socket_ICMP, 2, 0) < 0) {
        perror("set_socket_timeout");
        close(socket_UDP);
        close(socket_ICMP);
        return -1;
    }

    // Intestazione tabella
    printf("+-----+-----------------+------------+\n");
    printf("| Hop | IP              | RTT (ms)   |\n");
    printf("+-----+-----------------+------------+\n");

    for (int ttl = 1; ttl <= MAX_HOPS; ttl++) {
        // pacchetto attraversa solo ttl router, e quando scade router manda ICMP time exceeded
        if (set_socket_ttl(socket_UDP, ttl) < 0) {
            perror("e: set_socket_ttl");
            break;
        }

        dest_addr.sin_port = htons(BASE_PORT + ttl); // cambia porta di destinazione

        struct timeval start, end;
        gettimeofday(&start, NULL);

        // invia pacchetto udp
        if (send_udp_packet(socket_UDP, &dest_addr) < 0) {
            perror("e: send_udp_packet");
            break;
        }

        char buf[512];
        struct sockaddr_in recv_addr;
        socklen_t addr_len = sizeof(recv_addr);
        
        ssize_t recv_bytes = receive_icmp_packet(socket_ICMP, buf, sizeof(buf), &recv_addr, &addr_len);

        gettimeofday(&end, NULL);

        if (recv_bytes < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // timeout, nessuna risposta
                printf("| %3d | %-15s | %-10s |\n", ttl, "*", "         *");
            } else {
                perror("recvfrom");
                break;
            }
        } else {
            long rtt = timeval_diff_ms(&start, &end);

            char hop_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(recv_addr.sin_addr), hop_ip, sizeof(hop_ip));

            printf("| %3d | %-15s | %10ld |\n", ttl, hop_ip, rtt);

            if (strcmp(hop_ip, ip_str) == 0) {
                printf("+-----+-----------------+------------+\n");
                printf("Destinazione raggiunta.\n");
                break;
            }
        }
    }
    
    printf("+-----+-----------------+------------+\n");

    close(socket_UDP);
    close(socket_ICMP);
    return 0;
}
