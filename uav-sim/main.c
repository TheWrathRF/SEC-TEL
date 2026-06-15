#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

// One telemetry sample from the UAV. Kept simple for now - this is the data
// we'll later encrypt and push out over UDP.
struct telemetry {
    long long timestamp;   // unix time, seconds
    double    latitude;
    double    longitude;
    float     altitude;    // meters
    float     speed;       // m/s
    float     battery;     // percent, 0-100
};

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
    srand((unsigned)time(NULL));

    struct telemetry t;
    t.timestamp = (long long)time(NULL);
    t.latitude  = 41.015137;   // start somewhere over Istanbul
    t.longitude = 28.979530;
    t.altitude  = 0.0f;
    t.speed     = 0.0f;
    t.battery   = 100.0f;

    printf("UAV simulator started, generating telemetry...\n");

    while (1) {
        printf("[%lld] lat=%.5f lon=%.5f alt=%.1fm spd=%.1fm/s batt=%.1f%%\n",
               t.timestamp, t.latitude, t.longitude,
               t.altitude, t.speed, t.battery);
        fflush(stdout);

        step(&t);
        Sleep(1000);   // 1 Hz telemetry
    }

    return 0;
}
