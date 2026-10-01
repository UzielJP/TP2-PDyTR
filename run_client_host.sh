#!/bin/bash
# run_client_host.sh - Script del cliente que se ejecuta en el HOST (Punto 4b).
#
# Requiere: gcc instalado en el host (o WSL/MinGW en Windows).
# Se conecta al servidor que corre en la VM a través del forwarded_port
# en localhost:5555 (timed) y localhost:5556 (pingpong).
#
# Si estás en Windows sin gcc nativo, podés ejecutar esto desde WSL o
# desde una de las VMs apuntando a la IP del host.
#
# Uso: bash run_client_host.sh

set -e

SERVER_IP="127.0.0.1"
PORT_TIMED=5555
PORT_PP=5556
RESULTS="./results"
SIZES="10 100 1000 10000 100000 1000000"
MAX_RETRIES=30
RETRY_DELAY=2

mkdir -p "$RESULTS"

# Compilar los programas cliente (si no están compilados)
echo "[host] Compilando programas cliente..."
gcc -O2 -o client_timed client_timed.c 2>/dev/null || echo "[host] client_timed ya compilado o error de compilación"
gcc -O2 -o client_pingpong client_pingpong.c 2>/dev/null || echo "[host] client_pingpong ya compilado o error de compilación"

# Función de espera activa para resolver asincronismo
wait_for_port() {
    local host=$1
    local port=$2
    local retries=0

    echo "[wait] Esperando que $host:$port esté disponible..."
    while [ $retries -lt $MAX_RETRIES ]; do
        if (echo > /dev/tcp/$host/$port) 2>/dev/null; then
            echo "[wait] $host:$port está listo."
            return 0
        fi
        retries=$((retries + 1))
        echo "[wait] Intento $retries/$MAX_RETRIES - reintentando en ${RETRY_DELAY}s..."
        sleep $RETRY_DELAY
    done
    echo "[ERROR] $host:$port no respondió."
    exit 1
}

wait_for_port "$SERVER_IP" $PORT_TIMED
wait_for_port "$SERVER_IP" $PORT_PP

echo ""
echo "=========================================="
echo "[host] EXPERIMENTOS DE TIEMPOS (write/read)"
echo "=========================================="

for n in $SIZES; do
    echo "--- Enviando $n bytes ---"
    ./client_timed "$SERVER_IP" $PORT_TIMED $n "$RESULTS/cli_host_timed.csv"
    sleep 0.5
done

echo ""
echo "=========================================="
echo "[host] EXPERIMENTOS PING-PONG (RTT)"
echo "=========================================="

for n in $SIZES; do
    echo "--- Ping-pong con $n bytes ---"
    ./client_pingpong "$SERVER_IP" $PORT_PP $n "$RESULTS/cli_host_pingpong.csv"
    sleep 0.5
done

echo ""
echo "=========================================="
echo "[host] Todos los experimentos completados."
echo "  Resultados en: $RESULTS/"
echo "=========================================="
ls -la "$RESULTS/"
