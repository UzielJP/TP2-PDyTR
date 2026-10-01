/*
 * client_pingpong.c - Cliente TCP "ping-pong" (Punto 3, TP2).
 *
 * Protocolo:
 *   1. Envía 4 bytes (uint32_t en network order) con el total de bytes.
 *   2. Envía los N bytes (buffer fijo, patrón A-Z).
 *   3. Espera la recepción de los mismos N bytes de vuelta (eco).
 *   4. Mide el RTT total (desde antes del write hasta después de recibir
 *      todos los bytes de vuelta) y calcula RTT / 2.
 *
 * Uso: ./client_pingpong <ip_servidor> <puerto> <bytes> <archivo_csv>
 *   bytes: 10, 100, 1000, 10000, 100000 o 1000000
 *
 * CSV de salida: bytes_totales,rtt_us,rtt_half_us
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

static double elapsed_us(struct timespec t0, struct timespec t1) {
    return (t1.tv_sec - t0.tv_sec) * 1e6 + (t1.tv_nsec - t0.tv_nsec) / 1e3;
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Uso: %s <ip_servidor> <puerto> <bytes> <archivo_csv>\n",
                argv[0]);
        exit(1);
    }
    const char *ip = argv[1];
    int port = atoi(argv[2]);
    uint32_t total = (uint32_t)strtoul(argv[3], NULL, 10);
    FILE *csv = fopen(argv[4], "a");
    if (!csv) { perror("fopen"); exit(1); }

    /* Buffer fijo, asignado en el programa (sin leer de teclado) */
    char *data = malloc(total);
    if (!data) { perror("malloc"); exit(1); }
    for (uint32_t i = 0; i < total; i++) data[i] = (char)('A' + (i % 26));

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &servaddr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect"); exit(1);
    }

    /* Aviso previo: cuantos bytes se van a enviar */
    uint32_t total_net = htonl(total);
    write(sockfd, &total_net, sizeof(total_net));

    /* === Inicio de medición RTT === */
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    /* Enviar los N bytes */
    uint32_t sent = 0;
    while (sent < total) {
        ssize_t n = write(sockfd, data + sent, total - sent);
        if (n < 0) { perror("write"); break; }
        sent += (uint32_t)n;
    }

    /* Recibir los N bytes de vuelta (eco del servidor) */
    char *recv_buf = malloc(total);
    if (!recv_buf) { perror("malloc recv"); exit(1); }
    uint32_t received = 0;
    while (received < total) {
        ssize_t n = read(sockfd, recv_buf + received, total - received);
        if (n <= 0) break;
        received += (uint32_t)n;
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    /* === Fin de medición RTT === */

    double rtt_us = elapsed_us(t0, t1);
    double rtt_half = rtt_us / 2.0;

    fprintf(csv, "%u,%.3f,%.3f\n", total, rtt_us, rtt_half);
    fflush(csv);

    printf("[client_pp] total=%u enviados=%u recibidos=%u  RTT=%.2f us  RTT/2=%.2f us\n",
           total, sent, received, rtt_us, rtt_half);

    close(sockfd);
    free(data);
    free(recv_buf);
    fclose(csv);
    return 0;
}
