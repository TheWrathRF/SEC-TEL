#include <string.h>
#include "telemetry.h"
#include "aes.h"
#include "crc32.h"

// 16-byte AES key, must match the ground station
static const uint8_t aes_key[16] = {
    's','e','c','t','e','l','s','e','c','r','e','t','k','e','y','1'
};

int build_packet(const struct telemetry *t, unsigned char *out) {
    int plain = sizeof(struct telemetry);     // 36
    int padded = (plain / 16 + 1) * 16;       // 48, PKCS7 always adds at least one byte
    int pad = padded - plain;

    memcpy(out, t, plain);
    for (int i = plain; i < padded; i++)
        out[i] = (unsigned char)pad;          // PKCS7 padding

    // encrypt block by block (ECB)
    struct AES_ctx ctx;
    AES_init_ctx(&ctx, aes_key);
    for (int i = 0; i < padded; i += 16)
        AES_ECB_encrypt(&ctx, out + i);

    // crc over the encrypted bytes, appended little-endian
    unsigned int crc = crc32(out, padded);
    out[padded]     = (unsigned char)(crc);
    out[padded + 1] = (unsigned char)(crc >> 8);
    out[padded + 2] = (unsigned char)(crc >> 16);
    out[padded + 3] = (unsigned char)(crc >> 24);

    return padded + 4;
}
