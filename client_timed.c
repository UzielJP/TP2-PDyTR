/*
 * client_timed.c - Cliente TCP que envía un buffer de N bytes (asignados
 * directamente en el programa, sin leer de teclado ni mostrar en pantalla)
 * y mide el tiempo de write().
 *
 * El envío se hace en una única llamada a write() salvo que el valor de
 * retorno indique que quedaron datos pendientes (escritura parcial), en
 * cuyo caso se reintenta sólo con el resto no enviado.
 *
 * Uso: ./client_timed <ip_servidor> <puerto> <bytes> <archivo_csv>
 *   bytes: 10, 100, 1000, 10000, 100000 o 1000000 (ejercicio 3/4)
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
        fprintf(stderr, "Uso: %s <ip_servidor> <puerto> <bytes> <archivo_csv>\n", argv[0]);
        exit(1);
    }
    const char *ip = argv[1];
    int port = atoi(argv[2]);
    uint32_t total = (uint32_t)strtoul(argv[3], NULL, 10);
    FILE *csv = fopen(argv[4], "a");
    if (!csv) { perror("fopen"); exit(1); }

    /* Buffer fijo, asignado en el programa (no se lee de teclado) */
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

    uint32_t sent = 0;
    int call = 0;
    while (sent < total) {
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        ssize_t n = write(sockfd, data + sent, total - sent);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        if (n < 0) { perror("write"); break; }
        call++;
        sent += (uint32_t)n;
        fprintf(csv, "%u,%d,%zd,%.3f\n", total, call, n, elapsed_us(t0, t1));
        /* Si n == total - sent_anterior, se envio todo en una sola llamada. */
    }
    fflush(csv);
    printf("[client] total=%u enviados=%u en %d llamadas a write()\n", total, sent, call);

    close(sockfd);
    free(data);
    fclose(csv);
    return 0;
}
