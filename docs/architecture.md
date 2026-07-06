# System Architecture

SEC-TEL is a secure telemetry link with an electronic-warfare simulation on top of it.
A UAV sends encrypted flight data to a ground station over a UDP link that hops between
ports; a jammer tries to disrupt that link; everything is stored in PostgreSQL and shown
in Grafana.

```
   +-------------------+     encrypted telemetry over      +--------------------+
   |   UAV SIMULATOR   |         hopping UDP link          |   GROUND STATION   |
   |        (C)        | ===============================>> |       (Java)       |
   |-------------------|   dest port hops 5000-5100        |--------------------|
   | generate flight   |   every 5s (shared PRNG seed)     | hops its listening |
   |   data            |                                   |   port in lockstep |
   | AES-128 encrypt   |                ^                   | verify CRC-32      |
   | append CRC-32     |                |                   | AES-128 decrypt    |
   | hop the UDP port  |          intercept /               +---------+----------+
   +-------------------+          corrupt packets                     |
                                       |                       valid  |  corrupt
                          +------------+-----------+              +----+----+
                          |     JAMMER (Python)    |              |         |
                          |------------------------|              v         v
                          | sniff / bit-flip /     |        +---------+ +---------+
                          | noise flood            |        | clean_  | | attack_ |
                          +------------------------+        |telemetry| |  logs   |
                                                            +----+----+ +----+----+
                                                                 |           |
                                                                 +-----+-----+
                                                                       |
                                                             +-------------------+
                                                             |    PostgreSQL     |
                                                             |    (sectel_db)    |
                                                             +---------+---------+
                                                                       |
                                                                       v
                                                             +-------------------+
                                                             |      GRAFANA      |
                                                             | altitude / speed  |
                                                             | battery gauge     |
                                                             | EW warning panel  |
                                                             | attack log table  |
                                                             +-------------------+
```

## Components

- **UAV simulator (C, `uav-sim/`)** - generates flight telemetry, encrypts it with
  AES-128, appends a CRC-32, and sends it over UDP. The destination port changes every
  5 seconds based on a shared pseudo-random sequence.
- **Ground station (Java, `ground-station/`)** - runs the same port-hopping sequence so
  it listens on the right port, checks each packet's CRC-32, decrypts the valid ones and
  stores them, and logs the corrupted ones. Inserts run on a small thread + connection
  pool so bursts don't stall reception.
- **Jammer (Python, `jammer/`)** - the electronic-warfare attacker. It can sniff a port,
  bit-flip intercepted packets, or flood a port with random noise.
- **PostgreSQL (`db/`)** - `clean_telemetry` for valid flight data, `attack_logs` for
  corrupted/jammed packets.
- **Grafana (`grafana/`)** - dashboard reading from PostgreSQL.

## Packet format

```
[ AES-128 encrypted payload (48 bytes) ][ CRC-32 (4 bytes) ]   = 52 bytes
```

The payload is the 36-byte telemetry struct (timestamp, latitude, longitude, altitude,
speed, battery) PKCS7-padded to 48 bytes before encryption. The CRC-32 is computed over
the encrypted bytes, so any tampering is caught before decryption.

## Data flow

1. The UAV builds a telemetry sample, encrypts it, appends the CRC-32, and sends it to the
   current hop port.
2. The ground station, listening on the same port, checks the CRC.
3. If the CRC matches, it decrypts the payload and inserts a row into `clean_telemetry`.
4. If the CRC fails (corrupted or jammed), it writes a row into `attack_logs`.
5. Grafana queries both tables and shows live telemetry and any detected attacks.

## Defenses

- **AES-128** keeps the telemetry confidential.
- **CRC-32** detects any corruption or tampering of a packet.
- **Port hopping** means a jammer stuck on one port only affects the link while the hop
  happens to be on that port; the next hop moves the link out from under it.
