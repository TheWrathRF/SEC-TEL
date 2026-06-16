# SEC-TEL

Secure Telemetry & Electronic Warfare Simulation.

A UAV simulator written in C produces flight telemetry, encrypts it with AES-128,
appends a CRC-32 and transmits it over UDP using a port-hopping scheme. A Java ground
station receives, validates, decrypts and stores the data in PostgreSQL. A Python
module simulates a jammer that attempts to corrupt the data link, and Grafana
visualizes the live telemetry and any detected attacks.

## uav-sim (C)

Generates flight telemetry - timestamp, latitude, longitude, altitude, speed and
battery level - and sends it as a UDP packet to the ground station once per second.
Default destination is 127.0.0.1:5000.

### Build & run

```
cd uav-sim
make
./uav-sim.exe
```

Without make:

```
gcc -Wall main.c -o uav-sim.exe -lws2_32
```
