#!/bin/bash
# run_server.sh - Script del servidor para los experimentos del Punto 4a (VM-a-VM).
#
# Lanza el servidor timed (puerto 5555) y el servidor pingpong (puerto 5556)
# y espera a que los clientes se conecten.
# Los resultados del server_timed se guardan en /vagrant/results/srv_vm_timed.csv
#
# Uso: bash run_server.sh  (se ejecuta automáticamente desde el Vagrantfile)

set -e

RESULTS="/vagrant/results"
mkdir -p "$RESULTS"

echo "=========================================="
echo "[run_server] Iniciando servidores..."
echo "  server_timed   -> puerto 5555"
echo "  server_pingpong -> puerto 5556"
echo "=========================================="

cd /vagrant

# Lanzar server_timed en background
./server_timed 5555 "$RESULTS/srv_vm_timed.csv" &
PID_TIMED=$!

# Lanzar server_pingpong en background
./server_pingpong 5556 &
PID_PP=$!

echo "[run_server] PIDs: timed=$PID_TIMED  pingpong=$PID_PP"

# Señalizar que los servidores están listos
# (el cliente chequea la existencia de este archivo Y la disponibilidad del puerto)
echo "ready" > /vagrant/.server_ready

# Esperar a que terminen (Ctrl+C para matar)
wait $PID_TIMED $PID_PP 2>/dev/null || true

echo "[run_server] Servidores terminados."
