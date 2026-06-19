#include "ports.h"

// predefined seed - the ground station uses the same one to stay in sync
#define HOP_SEED 0x5eed1234u

int hop_port(long long slot) {
    unsigned int r = (unsigned int)slot ^ HOP_SEED;
    r = r * 1103515245u + 12345u;          // classic LCG step
    return PORT_BASE + (int)((r >> 16) % PORT_SPAN);
}
