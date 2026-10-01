/*
 * ClientPingPong.java - Cliente TCP "ping-pong" (Punto 3, TP2).
 *
 * Protocolo:
 *   1. Envía 4 bytes (int) con el total de bytes.
 *   2. Envía los N bytes (buffer fijo, patrón A-Z).
 *   3. Espera la recepción de los mismos N bytes de vuelta (eco).
 *   4. Mide el RTT total y calcula RTT / 2.
 *
 * Uso: java ClientPingPong <ip_servidor> <puerto> <bytes> <archivo_csv>
 *
 * CSV de salida: bytes_totales,rtt_us,rtt_half_us
 */

import java.io.*;
import java.net.*;

public class ClientPingPong {
    public static void main(String[] args) throws IOException {
        if (args.length < 4) {
            System.err.println("Uso: java ClientPingPong <ip_servidor> <puerto> <bytes> <archivo_csv>");
            System.exit(1);
        }
        String ip = args[0];
        int port = Integer.parseInt(args[1]);
        int total = Integer.parseInt(args[2]);
        PrintWriter csv = new PrintWriter(new FileWriter(args[3], true));

        /* Buffer fijo, asignado en el programa (sin leer de teclado) */
        byte[] data = new byte[total];
        for (int i = 0; i < total; i++) data[i] = (byte) ('A' + (i % 26));

        Socket socket = new Socket(ip, port);
        OutputStream out = socket.getOutputStream();
        InputStream in = socket.getInputStream();
        DataOutputStream dout = new DataOutputStream(out);

        /* Aviso previo: cuantos bytes se van a enviar */
        dout.writeInt(total);

        /* === Inicio de medición RTT === */
        long t0 = System.nanoTime();

        /* Enviar los N bytes */
        out.write(data);
        out.flush();

        /* Recibir los N bytes de vuelta (eco del servidor) */
        byte[] recvBuf = new byte[total];
        int received = 0;
        while (received < total) {
            int n = in.read(recvBuf, received, total - received);
            if (n <= 0) break;
            received += n;
        }

        long t1 = System.nanoTime();
        /* === Fin de medición RTT === */

        double rttUs = (t1 - t0) / 1000.0;
        double rttHalf = rttUs / 2.0;

        csv.printf("%d,%.3f,%.3f%n", total, rttUs, rttHalf);
        csv.flush();

        System.out.printf("[client_pp] total=%d enviados=%d recibidos=%d  RTT=%.2f us  RTT/2=%.2f us%n",
                total, total, received, rttUs, rttHalf);

        socket.close();
        csv.close();
    }
}
