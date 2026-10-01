/*
 * ServerTimed.java - Servidor TCP que recibe N bytes y mide el tiempo
 * de cada llamada a read() (buffer de lectura de 255 bytes).
 * Escribe un CSV: bytesTotales,nroLlamada,bytesLeidos,tiempoMicrosegundos
 *
 * Uso: java ServerTimed <puerto> <archivo_csv>
 */

import java.io.*;
import java.net.*;

public class ServerTimed {
    static final int READ_CHUNK = 255;

    public static void main(String[] args) throws IOException {
        if (args.length < 2) {
            System.err.println("Uso: java ServerTimed  ");
            System.exit(1);
        }
        int port = Integer.parseInt(args[0]);
        PrintWriter csv = new PrintWriter(new FileWriter(args[1], true));

        ServerSocket serverSocket = new ServerSocket(port);
        System.out.println("[server] escuchando en " + port + ", resultados en " + args[1]);

        while (true) {
            Socket socket = serverSocket.accept();
            System.out.println("[server] cliente conectado: " + socket.getRemoteSocketAddress());
            
            try {
                InputStream in = socket.getInputStream();
                DataInputStream din = new DataInputStream(in);
                int total = din.readInt(); // el cliente informa cuantos bytes va a mandar

                byte[] buffer = new byte[READ_CHUNK];
                int received = 0, call = 0;
                while (received < total) {
                    long t0 = System.nanoTime();
                    int n = in.read(buffer);
                    long t1 = System.nanoTime();
                    if (n <= 0) break;
                    call++;
                    received += n;
                    csv.printf("%d,%d,%d,%.3f%n", total, call, n, (t1 - t0) / 1000.0);
                }
                csv.flush();
                System.out.println("[server] total=" + total + " recibidos=" + received
                        + " en " + call + " llamadas a read()");
            } catch (EOFException e) {
                System.out.println("[server] cliente desconectado (ping de verificacion).");
            } catch (IOException e) {
                System.out.println("[server] error de I/O: " + e.getMessage());
            } finally {
                socket.close();
            }
        }
    }
}