#ifndef TELEMETRY_H
#define TELEMETRY_H

// a single telemetry sample produced by the UAV
#pragma pack(push, 1)
struct telemetry {
    long long timestamp;   // unix time, seconds
    double    latitude;
    double    longitude;
    float     altitude;    // meters
    float     speed;       // m/s
    float     battery;     // percent, 0-100
};
#pragma pack(pop)

// 36-byte struct -> PKCS7 padded to 48 -> + 4 byte CRC = 52
#define PACKET_LEN 52

// build the wire packet from a sample (encrypt + crc), returns its length
int build_packet(const struct telemetry *t, unsigned char *out);

#endif
