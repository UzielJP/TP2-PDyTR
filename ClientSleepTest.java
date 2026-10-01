/*
 * ClientSleepTest.java - Punto 5 del TP2: experimento con sleep en Java.
 *
 * Variante de ClientPingPong que permite insertar un Thread.sleep() de
 * N segundos en distintos puntos de la ejecución para analizar el
 * impacto del desfase temporal en las mediciones.
 *
 * Modos de operación (argumento <modo>):
 *   0 = Sin sleep (referencia / baseline)
 *   1 = Sleep de N segundos ANTES de crear el Socket (= antes de connect)
 *   2 = Sleep de N segundos DESPUÉS de connect pero ANTES de write
 *   3 = Sleep de N segundos DESPUÉS de write pero ANTES de read
 *
 * Uso: java ClientSleepTest <ip> <puerto> <bytes> <modo> <segundos_sleep> <archivo_csv>
 *
 * CSV: bytes,modo,sleep_s,connect_us,write_us,read_us,rtt_us,rtt_half_us
 */

import java.io.*;
import java.net.*;

public class ClientSleepTest {
    public static void main(String[] args) throws Exception {
        if (args.length < 6) {
            System.err.println(
                "Uso: java ClientSleepTest <ip> <puerto> <bytes> <modo> <segundos_sleep> <archivo_csv>\n" +
                "  modo 0: sin sleep (baseline)\n" +
                "  modo 1: sleep antes de connect()\n" +
                "  modo 2: sleep después de connect(), antes de write()\n" +
                "  modo 3: sleep después de write(), antes de read()");
            System.exit(1);
        }
        String ip = args[0];
        int port = Integer.parseInt(args[1]);
        int total = Integer.parseInt(args[2]);
        int modo = Integer.parseInt(args[3]);
        int sleepSecs = Integer.parseInt(args[4]);
        PrintWriter csv = new PrintWriter(new FileWriter(args[5], true));

        /* Buffer fijo */
        byte[] data = new byte[total];
        for (int i = 0; i < total; i++) data[i] = (byte) ('A' + (i % 26));

        /* MODO 1: Sleep ANTES del connect() */
        if (modo == 1) {
            System.out.printf("[sleep_test] Modo 1: durmiendo %d segundos ANTES de connect()...%n",
                    sleepSecs);
            Thread.sleep(sleepSecs * 1000L);
        }

        /* --- Connect --- */
        long tc0 = System.nanoTime();
        Socket socket = new Socket(ip, port);
        long tc1 = System.nanoTime();
        double tConnect = (tc1 - tc0) / 1000.0;

        OutputStream out = socket.getOutputStream();
        InputStream in = socket.getInputStream();
        DataOutputStream dout = new DataOutputStream(out);

        /* MODO 2: Sleep DESPUÉS del connect(), ANTES del write() */
        if (modo == 2) {
            System.out.printf("[sleep_test] Modo 2: durmiendo %d segundos DESPUÉS de connect(), ANTES de write()...%n",
                    sleepSecs);
            Thread.sleep(sleepSecs * 1000L);
        }

        /* Aviso previo: cuantos bytes se van a enviar */
        dout.writeInt(total);

        /* --- Write --- */
        long tw0 = System.nanoTime();
        out.write(data);
        out.flush();
        long tw1 = System.nanoTime();
        double tWrite = (tw1 - tw0) / 1000.0;

        /* MODO 3: Sleep DESPUÉS del write(), ANTES del read() */
        if (modo == 3) {
            System.out.printf("[sleep_test] Modo 3: durmiendo %d segundos DESPUÉS de write(), ANTES de read()...%n",
                    sleepSecs);
            Thread.sleep(sleepSecs * 1000L);
        }

        /* --- Read (recibir eco completo) --- */
        byte[] recvBuf = new byte[total];
        int received = 0;

        long tr0 = System.nanoTime();
        while (received < total) {
            int n = in.read(recvBuf, received, total - received);
            if (n <= 0) break;
            received += n;
        }
        long tr1 = System.nanoTime();
        double tRead = (tr1 - tr0) / 1000.0;

        /* RTT = write + read (sin contar el sleep intermedio) */
        double rtt = tWrite + tRead;
        double rttHalf = rtt / 2.0;

        csv.printf("%d,%d,%d,%.3f,%.3f,%.3f,%.3f,%.3f%n",
                total, modo, sleepSecs, tConnect, tWrite, tRead, rtt, rttHalf);
        csv.flush();

        System.out.printf("[sleep_test] bytes=%d modo=%d sleep=%ds%n", total, modo, sleepSecs);
        System.out.printf("  connect=%.2f us  write=%.2f us  read=%.2f us%n",
                tConnect, tWrite, tRead);
        System.out.printf("  RTT=%.2f us  RTT/2=%.2f us%n", rtt, rttHalf);

        socket.close();
        csv.close();
    }
}
