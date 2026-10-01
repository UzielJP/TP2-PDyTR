/*
 * server_timed.c - Servidor TCP que recibe N bytes (definidos por el
 * cliente) y mide el tiempo de CADA llamada a read().
 *
 * El buffer de lectura es de 255 bytes (ejercicio 4b), por lo que para
 * mensajes grandes hacen falta varias llamadas a read(); se guarda el
 * tiempo de cada una en un CSV: bytes_totales,nro_llamada,bytes_leidos,tiempo_us
 *
 * Uso: ./server_timed <puerto> <archivo_csv>
 * El servidor atiende conexiones en un bucle infinito (Ctrl+C para salir),
 * a diferencia del ejemplo "clásico" de un solo tiro.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define READ_CHUNK 255

static double elapsed_us(struct timespec t0, struct timespec t1) {
    return (t1.tv_sec - t0.tv_sec) * 1e6 + (t1.tv_nsec - t0.tv_nsec) / 1e3;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <puerto> <archivo_csv>\n", argv[0]);
        exit(1);
    }
    int port = atoi(argv[1]);
    FILE *csv = fopen(argv[2], "a");
    if (!csv) { perror("fopen"); exit(1); }

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
    printf("[server] escuchando en %d, resultados en %s\n", port, argv[2]);

    while (1) {
        struct sockaddr_in cliaddr;
        socklen_t clilen = sizeof(cliaddr);
        int connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        if (connfd < 0) { perror("accept"); continue; }
        printf("[server] cliente conectado: %s:%d\n",
               inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

        /* Primero el cliente informa cuantos bytes va a enviar (4 bytes, network order) */
        uint32_t total_net;
        ssize_t n0 = read(connfd, &total_net, sizeof(total_net));
        if (n0 != sizeof(total_net)) { close(connfd); continue; }
        uint32_t total = ntohl(total_net);

        char buffer[READ_CHUNK];
        uint32_t received = 0;
        int call = 0;
        while (received < total) {
            struct timespec t0, t1;
            clock_gettime(CLOCK_MONOTONIC, &t0);
            ssize_t n = read(connfd, buffer, READ_CHUNK);
            clock_gettime(CLOCK_MONOTONIC, &t1);
            if (n <= 0) break;
            call++;
            received += (uint32_t)n;
            fprintf(csv, "%u,%d,%zd,%.3f\n", total, call, n, elapsed_us(t0, t1));
        }
        fflush(csv);
        printf("[server] total=%u recibidos=%u en %d llamadas a read()\n",
               total, received, call);
        close(connfd);
    }

    fclose(csv);
    close(listenfd);
    return 0;
}
