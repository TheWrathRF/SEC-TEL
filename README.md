# SEC-TEL

Internship project - secure telemetry & electronic warfare simulation.

The idea: a UAV simulator (C) produces flight telemetry, encrypts it and sends it
over UDP while hopping between ports. A Java ground station receives, validates and
stores the data in PostgreSQL, a Python "jammer" tries to corrupt the link, and
Grafana shows everything on a dashboard.

## uav-sim (C)
For now this just generates dummy flight data (altitude, speed, position, battery)
and prints one sample per second. Encryption, CRC and UDP come in the next days.

### Build & run
```
cd uav-sim
make
./uav-sim.exe
```
or without make:
```
gcc -Wall main.c -o uav-sim.exe
```
