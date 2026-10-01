#!/bin/bash
# run_sleep_experiment.sh - Punto 5 del TP2.
#
# Ejecuta el experimento de desfase temporal con sleep(10) en distintos
# puntos del programa, comparando contra el baseline (sin sleep).
#
# Requiere:
#   - ServerPingPong corriendo en $SERVER_IP:$PORT
#   - ClientSleepTest compilado
#
# Uso: bash run_sleep_experiment.sh <ip_servidor> <puerto>

set -e

SERVER_IP="${1:-127.0.0.1}"
PORT="${2:-5556}"
RESULTS="./results"
SLEEP_SECS=10
CSV="$RESULTS/sleep_experiment.csv"

mkdir -p "$RESULTS"

# Encabezado del CSV
echo "bytes,modo,sleep_s,connect_us,write_us,read_us,rtt_us,rtt_half_us" > "$CSV"

echo "=========================================="
echo "[sleep_exp] Punto 5 - Experimento de desfase temporal"
echo "  Servidor: $SERVER_IP:$PORT"
echo "  Sleep: $SLEEP_SECS segundos"
echo "=========================================="

# Compilar si es necesario
javac ClientSleepTest.java 2>/dev/null || true

# Tamaños representativos (usamos un subconjunto para no demorar demasiado)
SIZES="100 10000 1000000"

for n in $SIZES; do
    echo ""
    echo "=== Tamaño: $n bytes ==="

    echo "--- Modo 0: Sin sleep (baseline) ---"
    java ClientSleepTest "$SERVER_IP" $PORT $n 0 0 "$CSV"
    sleep 1

    echo "--- Modo 1: Sleep $SLEEP_SECS s ANTES de connect() ---"
    java ClientSleepTest "$SERVER_IP" $PORT $n 1 $SLEEP_SECS "$CSV"
    sleep 1

    echo "--- Modo 2: Sleep $SLEEP_SECS s DESPUÉS de connect(), ANTES de write() ---"
    java ClientSleepTest "$SERVER_IP" $PORT $n 2 $SLEEP_SECS "$CSV"
    sleep 1

    echo "--- Modo 3: Sleep $SLEEP_SECS s DESPUÉS de write(), ANTES de read() ---"
    java ClientSleepTest "$SERVER_IP" $PORT $n 3 $SLEEP_SECS "$CSV"
    sleep 1
done

echo ""
echo "=========================================="
echo "[sleep_exp] Experimento completado."
echo "  Resultados en: $CSV"
echo "=========================================="

echo ""
echo "Contenido del CSV:"
cat "$CSV"