# SEC-TEL

Secure Telemetry & Electronic Warfare Simulation.

A UAV simulator written in C produces flight telemetry, encrypts it with AES-128,
appends a CRC-32 and transmits it over UDP using a port-hopping scheme. A Java ground
station receives, validates, decrypts and stores the data in PostgreSQL. A Python
module simulates a jammer that attempts to corrupt the data link, and Grafana
visualizes the live telemetry and any detected attacks.

## uav-sim (C)

Generates flight telemetry - timestamp, latitude, longitude, altitude, speed and
battery level - encrypts it with AES-128, appends a CRC-32, and sends the packet over
UDP to the ground station once per second. The destination port hops between 5000 and
5100 every 5 seconds (an LCG keyed on the current time slot, so the receiver can stay
in sync). Destination host is 127.0.0.1.

AES uses the vendored tiny-AES-c library (https://github.com/kokke/tiny-AES-c).

### Build & run

```
cd uav-sim
make
./uav-sim.exe
```

Without make:

```
gcc -Wall main.c packet.c crc32.c ports.c aes.c -o uav-sim.exe -lws2_32
```

## Database

The received data is stored in PostgreSQL. `db/schema.sql` creates two tables in a
database named `sectel_db`:

- `clean_telemetry` - valid, decrypted flight data
- `attack_logs` - packets that failed validation (corrupted / jammed)

Setup (default credentials are postgres / postgres):

```
psql -U postgres -c "CREATE DATABASE sectel_db;"
psql -U postgres -d sectel_db -f db/schema.sql
```

## ground-station (Java)

Follows the same port-hopping schedule as the sender, receives the UAV packets and
validates each one's CRC-32. Corrupted or jammed packets are recorded in the
`attack_logs` table. Uses plain JDBC; the driver jar is in `ground-station/lib`.

### Build & run

```
cd ground-station
javac -cp "lib/*" -d out src/*.java
java -cp "out;lib/*" Main
```
