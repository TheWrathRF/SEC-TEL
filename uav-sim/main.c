#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ground station address + port
#define GS_ADDR "127.0.0.1"
#define GS_PORT 5000

// One telemetry sample from the UAV. Kept simple for now - this is the data
// we'll later encrypt and push out over UDP.
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

// small random offset in [-scale, scale], used to make the data less robotic
static double jitter(double scale) {
    return ((double)rand() / RAND_MAX * 2.0 - 1.0) * scale;
}

// advance the flight by one second
static void step(struct telemetry *t) {
    t->timestamp += 1;

    float cruise = 120.0f;   // altitude we want to hold
    if (t->altitude < cruise) {
        t->altitude += 5.0f + (float)jitter(0.5);   // climb ~5 m/s
        if (t->altitude > cruise) t->altitude = cruise;
    } else {
        t->altitude += (float)jitter(0.8);          // hold, just some noise
    }

    if (t->speed < 15.0f) {
        t->speed += 0.7f + (float)jitter(0.2);
    } else {
        t->speed += (float)jitter(0.3);
    }

    // drift roughly north-east a little every tick
    t->latitude  += 0.00012 + jitter(0.00002);
    t->longitude += 0.00009 + jitter(0.00002);

    t->battery -= 0.05f;
    if (t->battery < 0.0f) t->battery = 0.0f;
}

int main(void) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        printf("socket failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = htons(GS_PORT);
    dest.sin_addr.s_addr = inet_addr(GS_ADDR);

    srand((unsigned)time(NULL));

    struct telemetry t;
    t.timestamp = (long long)time(NULL);
    t.latitude  = 41.015137;   // start somewhere over Istanbul
    t.longitude = 28.979530;
    t.altitude  = 0.0f;
    t.speed     = 0.0f;
    t.battery   = 100.0f;

    printf("UAV simulator started, sending telemetry to %s:%d\n", GS_ADDR, GS_PORT);

    while (1) {
        int n = sendto(sock, (char *)&t, sizeof(t), 0,
                       (struct sockaddr *)&dest, sizeof(dest));
        if (n == SOCKET_ERROR) {
            printf("sendto failed: %d\n", WSAGetLastError());
        } else {
            printf("[%lld] lat=%.5f lon=%.5f alt=%.1fm spd=%.1fm/s batt=%.1f%% -> %d bytes\n",
                   t.timestamp, t.latitude, t.longitude,
                   t.altitude, t.speed, t.battery, n);
        }
        fflush(stdout);

        step(&t);
        Sleep(1000);   // 1 Hz telemetry
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
