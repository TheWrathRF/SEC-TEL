#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "telemetry.h"
#include "ports.h"

// ground station address (the port changes by hopping, see ports.h)
#define GS_ADDR "127.0.0.1"

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
    dest.sin_addr.s_addr = inet_addr(GS_ADDR);
    // sin_port is set in the loop, it changes as the port hops

    srand((unsigned)time(NULL));

    struct telemetry t;
    t.timestamp = (long long)time(NULL);
    t.latitude  = 41.015137;   // start somewhere over Istanbul
    t.longitude = 28.979530;
    t.altitude  = 0.0f;
    t.speed     = 0.0f;
    t.battery   = 100.0f;

    printf("UAV simulator started, sending to %s, port hops %d-%d every %ds\n",
           GS_ADDR, PORT_BASE, PORT_BASE + PORT_SPAN - 1, HOP_SECONDS);

    unsigned char packet[PACKET_LEN];
    int cur_port = 0;

    while (1) {
        long long slot = (long long)time(NULL) / HOP_SECONDS;
        int port = hop_port(slot);
        if (port != cur_port) {
            cur_port = port;
            dest.sin_port = htons((unsigned short)port);
            printf("== hop -> port %d ==\n", port);
        }

        int len = build_packet(&t, packet);
        int n = sendto(sock, (char *)packet, len, 0,
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
