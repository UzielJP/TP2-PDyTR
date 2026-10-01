/*
 * client_sleep_test.c - Punto 5 del TP2: experimento con sleep(10).
 *
 * Este programa es una variante de client_pingpong.c que permite insertar
 * un sleep() de N segundos en distintos puntos de la ejecución para
 * analizar el impacto del desfase temporal en las mediciones.
 *
 * Modos de operación (argumento <modo>):
 *   0 = Sin sleep (referencia / baseline)
 *   1 = Sleep de N segundos ANTES del connect()
 *   2 = Sleep de N segundos DESPUÉS del connect() pero ANTES del write()
 *   3 = Sleep de N segundos DESPUÉS del write() pero ANTES del read()
 *
 * Uso: ./client_sleep_test <ip> <puerto> <bytes> <modo> <segundos_sleep> <archivo_csv>
 *
 * CSV de salida: bytes,modo,sleep_s,tiempo_connect_us,tiempo_write_us,tiempo_read_us,rtt_us,rtt_half_us
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
    if (argc < 7) {
        fprintf(stderr,
            "Uso: %s <ip> <puerto> <bytes> <modo> <segundos_sleep> <archivo_csv>\n"
            "  modo 0: sin sleep (baseline)\n"
            "  modo 1: sleep antes de connect()\n"
            "  modo 2: sleep después de connect(), antes de write()\n"
            "  modo 3: sleep después de write(), antes de read()\n",
            argv[0]);
        exit(1);
    }
    const char *ip = argv[1];
    int port = atoi(argv[2]);
    uint32_t total = (uint32_t)strtoul(argv[3], NULL, 10);
    int modo = atoi(argv[4]);
    int sleep_secs = atoi(argv[5]);
    FILE *csv = fopen(argv[6], "a");
    if (!csv) { perror("fopen"); exit(1); }

    /* Buffer fijo */
    char *data = malloc(total);
    if (!data) { perror("malloc"); exit(1); }
    for (uint32_t i = 0; i < total; i++) data[i] = (char)('A' + (i % 26));

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &servaddr.sin_addr);

    /* MODO 1: Sleep ANTES del connect() */
    if (modo == 1) {
        printf("[sleep_test] Modo 1: durmiendo %d segundos ANTES de connect()...\n",
               sleep_secs);
        sleep(sleep_secs);
    }

    /* --- Connect --- */
    struct timespec tc0, tc1;
    clock_gettime(CLOCK_MONOTONIC, &tc0);
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect"); exit(1);
    }
    clock_gettime(CLOCK_MONOTONIC, &tc1);
    double t_connect = elapsed_us(tc0, tc1);

    /* MODO 2: Sleep DESPUÉS del connect(), ANTES del write() */
    if (modo == 2) {
        printf("[sleep_test] Modo 2: durmiendo %d segundos DESPUÉS de connect(), ANTES de write()...\n",
               sleep_secs);
        sleep(sleep_secs);
    }

    /* Aviso previo: cuantos bytes se van a enviar */
    uint32_t total_net = htonl(total);
    write(sockfd, &total_net, sizeof(total_net));

    /* --- Write --- */
    struct timespec tw0, tw1;
    clock_gettime(CLOCK_MONOTONIC, &tw0);
    uint32_t sent = 0;
    while (sent < total) {
        ssize_t n = write(sockfd, data + sent, total - sent);
        if (n < 0) { perror("write"); break; }
        sent += (uint32_t)n;
    }
    clock_gettime(CLOCK_MONOTONIC, &tw1);
    double t_write = elapsed_us(tw0, tw1);

    /* MODO 3: Sleep DESPUÉS del write(), ANTES del read() */
    if (modo == 3) {
        printf("[sleep_test] Modo 3: durmiendo %d segundos DESPUÉS de write(), ANTES de read()...\n",
               sleep_secs);
        sleep(sleep_secs);
    }

    /* --- Read (recibir eco completo) --- */
    struct timespec tr0, tr1;
    char *recv_buf = malloc(total);
    if (!recv_buf) { perror("malloc recv"); exit(1); }
    uint32_t received = 0;

    clock_gettime(CLOCK_MONOTONIC, &tr0);
    while (received < total) {
        ssize_t n = read(sockfd, recv_buf + received, total - received);
        if (n <= 0) break;
        received += (uint32_t)n;
    }
    clock_gettime(CLOCK_MONOTONIC, &tr1);
    double t_read = elapsed_us(tr0, tr1);

    /* RTT = write + read (sin contar el sleep intermedio) */
    double rtt = t_write + t_read;
    double rtt_half = rtt / 2.0;

    fprintf(csv, "%u,%d,%d,%.3f,%.3f,%.3f,%.3f,%.3f\n",
            total, modo, sleep_secs, t_connect, t_write, t_read, rtt, rtt_half);
    fflush(csv);

    printf("[sleep_test] bytes=%u modo=%d sleep=%ds\n", total, modo, sleep_secs);
    printf("  connect=%.2f us  write=%.2f us  read=%.2f us\n",
           t_connect, t_write, t_read);
    printf("  RTT=%.2f us  RTT/2=%.2f us\n", rtt, rtt_half);

    close(sockfd);
    free(data);
    free(recv_buf);
    fclose(csv);
    return 0;
}
