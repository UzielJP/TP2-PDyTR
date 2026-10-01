#!/usr/bin/env python3
"""
plot_times_tp2.py - Grafica los resultados del TP2 (Puntos 2, 3 y 5).

Genera los siguientes gráficos:
  1. write_times.png      -> Tiempo de write() vs bytes (Punto 2a)
  2. read_times.png       -> Tiempo promedio de read() vs bytes (Punto 2b)
  3. pingpong_vs_timed.png -> Comparación RTT/2 (Punto 3) vs write del Punto 2
  4. sleep_impact.png     -> Impacto del sleep(10) en cada fase (Punto 5)

Uso:
  python3 plot_times_tp2.py <cli_timed.csv> <srv_timed.csv> <pingpong.csv> [sleep.csv]

CSVs esperados:
  cli_timed.csv:   bytes_totales,nro_llamada,bytes_transferidos,tiempo_us
  srv_timed.csv:   bytes_totales,nro_llamada,bytes_leidos,tiempo_us
  pingpong.csv:    bytes_totales,rtt_us,rtt_half_us
  sleep.csv:       bytes,modo,sleep_s,connect_us,write_us,read_us,rtt_us,rtt_half_us
"""
import sys
import csv
import collections
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt


def load_timed(path):
    rows = []
    with open(path) as f:
        for line in csv.reader(f):
            if not line or line[0].startswith("bytes"):
                continue
            total, call, n, us = int(line[0]), int(line[1]), int(line[2]), float(line[3])
            rows.append((total, call, n, us))
    return rows


def load_pingpong(path):
    rows = []
    with open(path) as f:
        for line in csv.reader(f):
            if not line or line[0].startswith("bytes"):
                continue
            total, rtt, rtt_half = int(line[0]), float(line[1]), float(line[2])
            rows.append((total, rtt, rtt_half))
    return rows


def load_sleep(path):
    rows = []
    with open(path) as f:
        for line in csv.reader(f):
            if not line or line[0].startswith("bytes"):
                continue
            b, modo, sl, conn, wr, rd, rtt, rtt_h = (
                int(line[0]), int(line[1]), int(line[2]),
                float(line[3]), float(line[4]), float(line[5]),
                float(line[6]), float(line[7])
            )
            rows.append((b, modo, sl, conn, wr, rd, rtt, rtt_h))
    return rows


def main():
    if len(sys.argv) < 4:
        print(f"Uso: {sys.argv[0]} <cli_timed.csv> <srv_timed.csv> <pingpong.csv> [sleep.csv]")
        sys.exit(1)

    cli_rows = load_timed(sys.argv[1])
    srv_rows = load_timed(sys.argv[2])
    pp_rows = load_pingpong(sys.argv[3])

    # ===== Gráfico 1: write() vs bytes (Punto 2a) =====
    write_time_by_total = collections.defaultdict(float)
    for total, _call, _n, us in cli_rows:
        write_time_by_total[total] += us
    sizes = sorted(write_time_by_total)
    write_times = [write_time_by_total[s] for s in sizes]

    plt.figure(figsize=(8, 5))
    plt.plot(sizes, write_times, marker="o", linewidth=2)
    plt.xscale("log")
    plt.yscale("log")
    plt.xlabel("Bytes enviados")
    plt.ylabel("Tiempo total de write() [µs]")
    plt.title("Punto 2a - Tiempo de write() vs cantidad de datos")
    plt.grid(True, which="both", ls="--", alpha=0.5)
    plt.savefig("write_times.png", dpi=150, bbox_inches="tight")
    print("Generado: write_times.png")

    # ===== Gráfico 2: read() promedio vs bytes (Punto 2b) =====
    read_time_by_total = collections.defaultdict(list)
    for total, _call, _n, us in srv_rows:
        read_time_by_total[total].append(us)
    sizes_r = sorted(read_time_by_total)
    avg_read = [sum(v) / len(v) for v in (read_time_by_total[s] for s in sizes_r)]

    plt.figure(figsize=(8, 5))
    plt.plot(sizes_r, avg_read, marker="o", color="darkorange", linewidth=2)
    plt.xscale("log")
    plt.xlabel("Bytes totales del mensaje")
    plt.ylabel("Tiempo PROMEDIO por llamada a read() [µs]")
    plt.title("Punto 2b - Tiempo promedio de cada read() (buffer fijo 255 bytes)")
    plt.grid(True, which="both", ls="--", alpha=0.5)
    plt.savefig("read_times.png", dpi=150, bbox_inches="tight")
    print("Generado: read_times.png")

    # ===== Gráfico 3: Comparación Ping-Pong RTT/2 vs write (Punto 3) =====
    pp_by_size = {}
    for total, rtt, rtt_half in pp_rows:
        pp_by_size[total] = rtt_half
    sizes_pp = sorted(pp_by_size)
    rtt_halves = [pp_by_size[s] for s in sizes_pp]

    # Filtrar write_times solo a los tamaños que coinciden
    common_sizes = sorted(set(sizes) & set(sizes_pp))
    write_common = [write_time_by_total[s] for s in common_sizes]
    pp_common = [pp_by_size[s] for s in common_sizes]

    plt.figure(figsize=(8, 5))
    plt.plot(common_sizes, write_common, marker="o", label="write() - Punto 2", linewidth=2)
    plt.plot(common_sizes, pp_common, marker="s", label="RTT/2 - Punto 3 (ping-pong)", linewidth=2)
    plt.xscale("log")
    plt.yscale("log")
    plt.xlabel("Bytes")
    plt.ylabel("Tiempo [µs]")
    plt.title("Punto 3 - Comparación: write() vs RTT/2 (ping-pong)")
    plt.legend()
    plt.grid(True, which="both", ls="--", alpha=0.5)
    plt.savefig("pingpong_vs_timed.png", dpi=150, bbox_inches="tight")
    print("Generado: pingpong_vs_timed.png")

    # ===== Gráfico 4: Impacto del sleep (Punto 5) - solo si hay CSV =====
    if len(sys.argv) >= 5:
        sleep_rows = load_sleep(sys.argv[4])

        modo_labels = {
            0: "Sin sleep (baseline)",
            1: "Sleep antes de connect()",
            2: "Sleep antes de write()",
            3: "Sleep antes de read()"
        }

        # Agrupar por tamaño y modo
        data_by_size = collections.defaultdict(dict)
        for b, modo, sl, conn, wr, rd, rtt, rtt_h in sleep_rows:
            data_by_size[b][modo] = {"connect": conn, "write": wr, "read": rd, "rtt": rtt}

        fig, axes = plt.subplots(1, len(data_by_size), figsize=(5 * len(data_by_size), 5), sharey=False)
        if len(data_by_size) == 1:
            axes = [axes]

        for ax, size in zip(axes, sorted(data_by_size)):
            modos = sorted(data_by_size[size])
            rtt_vals = [data_by_size[size][m]["rtt"] for m in modos]
            labels = [modo_labels.get(m, f"Modo {m}") for m in modos]

            bars = ax.bar(range(len(modos)), rtt_vals, color=["#2196F3", "#FF9800", "#4CAF50", "#F44336"])
            ax.set_xticks(range(len(modos)))
            ax.set_xticklabels([f"Modo {m}" for m in modos], rotation=45, ha="right")
            ax.set_ylabel("RTT (write+read) [µs]")
            ax.set_title(f"{size} bytes")
            ax.grid(axis="y", ls="--", alpha=0.5)

        fig.suptitle("Punto 5 - Impacto del sleep(10) en los tiempos de comunicación", fontsize=13)
        plt.tight_layout()
        plt.savefig("sleep_impact.png", dpi=150, bbox_inches="tight")
        print("Generado: sleep_impact.png")

    # ===== Resúmenes =====
    print("\n--- Resumen write() ---")
    for s, t in zip(sizes, write_times):
        print(f"  {s:>8} bytes -> {t:10.2f} µs total")

    print("\n--- Resumen read() (promedio por llamada) ---")
    for s, t in zip(sizes_r, avg_read):
        print(f"  {s:>8} bytes -> {t:10.2f} µs/llamada  ({len(read_time_by_total[s])} llamadas)")

    print("\n--- Resumen ping-pong RTT/2 ---")
    for s in sizes_pp:
        print(f"  {s:>8} bytes -> RTT/2 = {pp_by_size[s]:10.2f} µs")


if __name__ == "__main__":
    main()
