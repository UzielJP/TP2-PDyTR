/*
 * ServerPingPong.java - Servidor TCP "ping-pong" (Punto 3, TP2).
 *
 * Protocolo:
 *   1. Recibe 4 bytes (int, network order via DataInputStream) indicando
 *      cuántos bytes totales va a enviar el cliente.
 *   2. Lee esos N bytes completos.
 *   3. Reenvía los N bytes completos de vuelta al cliente (eco total).
 *
 * Uso: java ServerPingPong <puerto>
 */

import java.io.*;
import java.net.*;

public class ServerPingPong {
    public static void main(String[] args) throws IOException {
        if (args.length < 1) {
            System.err.println("Uso: java ServerPingPong <puerto>");
            System.exit(1);
        }
        int port = Integer.parseInt(args[0]);

        ServerSocket serverSocket = new ServerSocket(port);
        System.out.println("[server_pp] escuchando en puerto " + port);

        while (true) {
            Socket socket = serverSocket.accept();
            System.out.println("[server_pp] cliente conectado: "
                    + socket.getRemoteSocketAddress());

            InputStream in = socket.getInputStream();
            OutputStream out = socket.getOutputStream();

            try {
                /* 1) Leer cuántos bytes va a enviar el cliente */
                DataInputStream din = new DataInputStream(in);
                int total = din.readInt();

                /* 2) Recibir los N bytes completos */
                byte[] buffer = new byte[total];
                int received = 0;
                while (received < total) {
                    int n = in.read(buffer, received, total - received);
                    if (n <= 0) break;
                    received += n;
                }

                System.out.println("[server_pp] recibidos " + received + "/"
                        + total + " bytes, reenviando...");

                /* 3) Reenviar los mismos N bytes de vuelta (ping-pong) */
                out.write(buffer, 0, received);
                out.flush();

                System.out.println("[server_pp] reenviados " + received + " bytes");

            } catch (EOFException e) {
                System.out.println("[server_pp] cliente desconectado.");
            }

            socket.close();
        }
    }
}
