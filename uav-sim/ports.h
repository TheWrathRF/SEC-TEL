#ifndef PORTS_H
#define PORTS_H

#define PORT_BASE   5000
#define PORT_SPAN   101    // ports 5000..5100
#define HOP_SECONDS 5

// destination port for a given 5-second time slot (must match the ground station)
int hop_port(long long slot);

#endif
