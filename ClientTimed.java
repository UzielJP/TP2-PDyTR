/*
 * ClientTimed.java - Cliente TCP que envía un buffer de N bytes (fijado
 * en el programa) y mide el tiempo de write(). Una única llamada salvo
 * que OutputStream.write no admita escritura parcial explícita: en Java,
 * OutputStream.write(byte[]) bloquea hasta escribir todo el arreglo, por
 * lo que aquí se mide esa única llamada (a diferencia de C, donde sí puede
 * haber escrituras parciales que haya que reintentar).
 *
 * Uso: java ClientTimed <ip_servidor> <puerto> <bytes> <archivo_csv>
 */

import java.io.*;
import java.net.*;

public class ClientTimed {
    public static void main(String[] args) throws IOException {
        if (args.length < 4) {
            System.err.println("Uso: java ClientTimed <ip_servidor> <puerto> <bytes> <archivo_csv>");
            System.exit(1);
        }
        String ip = args[0];
        int port = Integer.parseInt(args[1]);
        int total = Integer.parseInt(args[2]);
        PrintWriter csv = new PrintWriter(new FileWriter(args[3], true));

        byte[] data = new byte[total];
        for (int i = 0; i < total; i++) data[i] = (byte) ('A' + (i % 26));

        Socket socket = new Socket(ip, port);
        OutputStream out = socket.getOutputStream();
        DataOutputStream dout = new DataOutputStream(out);
        dout.writeInt(total); // avisa cuantos bytes va a mandar

        long t0 = System.nanoTime();
        out.write(data); // una unica llamada: write(byte[]) escribe el arreglo completo
        out.flush();
        long t1 = System.nanoTime();

        csv.printf("%d,1,%d,%.3f%n", total, total, (t1 - t0) / 1000.0);
        csv.flush();
        System.out.println("[client] total=" + total + " enviados en 1 llamada a write()");

        socket.close();
    }
}
