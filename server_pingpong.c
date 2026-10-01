/*
 * server_pingpong.c - Servidor TCP "ping-pong" (Punto 3, TP2).
 *
 * Protocolo:
 *   1. Recibe 4 bytes (uint32_t en network order) indicando cuántos bytes
 *      totales va a enviar el cliente.
 *   2. Lee esos N bytes completos (en chunks de READ_CHUNK).
 *   3. Reenvía los N bytes completos de vuelta al cliente (eco total).
 *
 * De esta forma el cliente puede medir el RTT completo (ida + vuelta)
 * y estimar el tiempo en una dirección como RTT / 2.
 *
 * Uso: ./server_pingpong <puerto>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define READ_CHUNK 255

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <puerto>\n", argv[0]);
        exit(1);
    }
    int port = atoi(argv[1]);

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);
    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(listenfd, 5);
    printf("[server_pp] escuchando en puerto %d\n", port);

    while (1) {
        struct sockaddr_in cliaddr;
        socklen_t clilen = sizeof(cliaddr);
        int connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        if (connfd < 0) { perror("accept"); continue; }
        printf("[server_pp] cliente conectado: %s:%d\n",
               inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

        /* 1) Leer cuántos bytes va a enviar el cliente */
        uint32_t total_net;
        ssize_t n0 = read(connfd, &total_net, sizeof(total_net));
        if (n0 != sizeof(total_net)) { close(connfd); continue; }
        uint32_t total = ntohl(total_net);

        /* 2) Recibir los N bytes completos en un buffer dinámico */
        char *buffer = malloc(total);
        if (!buffer) { perror("malloc"); close(connfd); continue; }

        uint32_t received = 0;
        while (received < total) {
            ssize_t n = read(connfd, buffer + received, total - received);
            if (n <= 0) break;
            received += (uint32_t)n;
        }

        printf("[server_pp] recibidos %u/%u bytes, reenviando...\n",
               received, total);

        /* 3) Reenviar los mismos N bytes de vuelta (ping-pong) */
        uint32_t sent = 0;
        while (sent < received) {
            ssize_t n = write(connfd, buffer + sent, received - sent);
            if (n <= 0) break;
            sent += (uint32_t)n;
        }
        printf("[server_pp] reenviados %u bytes\n", sent);

        free(buffer);
        close(connfd);
    }

    close(listenfd);
    return 0;
}
