#include "crc32.h"

// standard CRC-32 (IEEE), same result as zlib and java.util.zip.CRC32
unsigned int crc32(const unsigned char *data, int len) {
    unsigned int crc = 0xFFFFFFFF;
    for (int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }
    return crc ^ 0xFFFFFFFF;
}
