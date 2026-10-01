#!/bin/bash
# run_client.sh - Script del cliente para los experimentos del Punto 4a (VM-a-VM).
#
# Resuelve el problema de asincronismo:
#   1. Espera a que el archivo /vagrant/.server_ready exista (señalización por archivo).
#   2. Verifica con un bucle de reintentos que el puerto TCP esté realmente
#      escuchando (usando /dev/tcp, ya que nc puede no estar instalado).
#
# Luego ejecuta:
#   - Los experimentos de tiempos con client_timed (write/read) para 10..10^6 bytes.
#   - Los experimentos de ping-pong con client_pingpong para 10..10^6 bytes.
#
# Uso: bash run_client.sh <ip_servidor>

set -e

if [ -z "$1" ]; then
    echo "Uso: $0 <ip_servidor>"
    exit 1
fi

SERVER_IP="$1"
PORT_TIMED=5555
PORT_PP=5556
RESULTS="/vagrant/results"
SIZES="10 100 1000 10000 100000 1000000"
MAX_RETRIES=30
RETRY_DELAY=2

mkdir -p "$RESULTS"

# =====================================================================
# Función: wait_for_port - Espera activa hasta que un puerto TCP esté
# escuchando. Este es el mecanismo que resuelve el asincronismo.
# Si el cliente arranca antes que el servidor, en lugar de fallar con
# "Connection refused", reintenta cada RETRY_DELAY segundos.
# =====================================================================
wait_for_port() {
    local host=$1
    local port=$2
    local retries=0

    echo "[wait] Esperando que $host:$port esté disponible..."
    while [ $retries -lt $MAX_RETRIES ]; do
        # Intentar abrir una conexión TCP usando bash built-in /dev/tcp
        if (echo > /dev/tcp/$host/$port) 2>/dev/null; then
            echo "[wait] $host:$port está listo."
            return 0
        fi
        retries=$((retries + 1))
        echo "[wait] Intento $retries/$MAX_RETRIES - puerto no disponible, reintentando en ${RETRY_DELAY}s..."
        sleep $RETRY_DELAY
    done

    echo "[ERROR] $host:$port no respondió después de $MAX_RETRIES intentos."
    exit 1
}

# =====================================================================
# Paso 1: Esperar señalización por archivo compartido
# =====================================================================
echo "[run_client] Esperando señalización del servidor (/vagrant/.server_ready)..."
retries=0
while [ ! -f /vagrant/.server_ready ] && [ $retries -lt $MAX_RETRIES ]; do
    retries=$((retries + 1))
    echo "[run_client] Servidor aún no listo, esperando... ($retries/$MAX_RETRIES)"
    sleep $RETRY_DELAY
done

if [ ! -f /vagrant/.server_ready ]; then
    echo "[ERROR] El archivo de señalización del servidor no apareció."
    exit 1
fi

# =====================================================================
# Paso 2: Verificar que los puertos TCP estén realmente escuchando
# =====================================================================
wait_for_port "$SERVER_IP" $PORT_TIMED
wait_for_port "$SERVER_IP" $PORT_PP

echo ""
echo "=========================================="
echo "[run_client] EXPERIMENTOS DE TIEMPOS (write/read)"
echo "  Servidor: $SERVER_IP:$PORT_TIMED"
echo "=========================================="

cd /vagrant

for n in $SIZES; do
    echo "--- Enviando $n bytes ---"
    ./client_timed "$SERVER_IP" $PORT_TIMED $n "$RESULTS/cli_vm_timed.csv"
    sleep 0.5
done

echo ""
echo "=========================================="
echo "[run_client] EXPERIMENTOS PING-PONG (RTT)"
echo "  Servidor: $SERVER_IP:$PORT_PP"
echo "=========================================="

for n in $SIZES; do
    echo "--- Ping-pong con $n bytes ---"
    ./client_pingpong "$SERVER_IP" $PORT_PP $n "$RESULTS/cli_vm_pingpong.csv"
    sleep 0.5
done

echo ""
echo "=========================================="
echo "[run_client] Todos los experimentos completados."
echo "  Resultados en: $RESULTS/"
echo "=========================================="

# Listar los resultados generados
ls -la "$RESULTS/"
