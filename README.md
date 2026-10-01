# TP2 — Programación Distribuida y Tiempo Real (Java)

## Requisitos

- **Java JDK 17+** (`sudo apt install default-jdk` en Ubuntu)
- **Vagrant** + **VirtualBox** (para puntos 1 y 4)
- **Python 3** con matplotlib (para gráficos)

## Compilar todo

```bash
javac *.java
```

---

## Ejercicio 1 — Vagrant con Ubuntu 24.04 LTS

**Archivos:**
- `Vagrantfile.punto1`

**Ejecución:**
```bash
cp Vagrantfile.punto1 Vagrantfile
vagrant up
vagrant ssh

# Dentro de la VM:
lsb_release -a
uname -r
java -version
```

---

## Ejercicio 2 — Experimentos entre dos PCs físicas

**Archivos:**
- `ServerTimed.java` — servidor que mide tiempos de cada `read()`
- `ClientTimed.java` — cliente que mide tiempo de `write()`

**Ejecución (PC A = servidor):**
```bash
javac ServerTimed.java
java ServerTimed 5555 srv_fisico.csv
```

**Ejecución (PC B = cliente):**
```bash
javac ClientTimed.java
for n in 10 100 1000 10000 100000 1000000; do
    java ClientTimed <IP_PC_A> 5555 $n cli_fisico.csv
done
```

---

## Ejercicio 3 — Ping-Pong (RTT / 2)

**Archivos:**
- `ServerPingPong.java` — servidor eco (recibe N bytes, reenvía N bytes)
- `ClientPingPong.java` — cliente que mide RTT completo

**Ejecución (servidor):**
```bash
javac ServerPingPong.java
java ServerPingPong 5556
```

**Ejecución (cliente):**
```bash
javac ClientPingPong.java
for n in 10 100 1000 10000 100000 1000000; do
    java ClientPingPong <IP_SERVIDOR> 5556 $n pingpong.csv
done
```

---

## Ejercicio 4 — Scripts de despliegue con Vagrant

**Archivos:**
- `Vagrantfile.punto4a` — 2 VMs (VM a VM, red privada)
- `Vagrantfile.punto4b` — 1 VM + host (forwarded_port)
- `run_server.sh` — compila y lanza servidores Java en background
- `run_client.sh` — espera al servidor + ejecuta todos los experimentos
- `run_client_host.sh` — cliente desde el host (escenario 4b)

**Ejecución escenario 4a (VM a VM):**
```bash
cp Vagrantfile.punto4a Vagrantfile
vagrant up
# Los experimentos corren automáticamente. Resultados en results/
```

**Ejecución escenario 4b (VM + host):**
```bash
cp Vagrantfile.punto4b Vagrantfile
vagrant up
# Desde el host:
bash run_client_host.sh
```

---

## Ejercicio 5 — Impacto del sleep / desfase temporal

**Archivos:**
- `ClientSleepTest.java` — cliente con `Thread.sleep()` en 4 modos
- `run_sleep_experiment.sh` — automatiza los 4 modos × 3 tamaños

**Ejecución manual:**
```bash
# Terminal 1: servidor pingpong
java ServerPingPong 5556

# Terminal 2: un modo específico
java ClientSleepTest <IP> 5556 <BYTES> <MODO> <SEGUNDOS_SLEEP> resultado.csv
# Ejemplo: modo 3, 1000 bytes, sleep de 10 segundos
java ClientSleepTest 127.0.0.1 5556 1000 3 10 resultado.csv
```

**Modos disponibles:**
| Modo | Sleep de N segundos... |
|------|------------------------|
| 0 | Sin sleep (baseline) |
| 1 | Antes de connect |
| 2 | Después de connect, antes de write |
| 3 | Después de write, antes de read |

**Ejecución automatizada (todos los modos):**
```bash
java ServerPingPong 5556 &
bash run_sleep_experiment.sh 127.0.0.1 5556
# Resultados en results/sleep_experiment.csv
```

---

## Gráficos

**Archivo:**
- `plot_times_tp2.py`

```bash
python3 plot_times_tp2.py cli_fisico.csv srv_fisico.csv pingpong.csv
```

---

## Informes

| Archivo | Contenido |
|---|---|
| `Practica2_Informe_Java.md` | Informe principal del TP2 (versión Java) |
| `Informe_Codigo_Java.md` | Explicación detallada de cada archivo Java |
