#include "stdint.h"

void generate_mac_address(uint8_t* device_id, uint8_t* mac_buffer)
{
    // Set the first octet to 0x02 to indicate a locally administered MAC address
    mac_buffer[0] = 0x02;

    // Generate the remaining 5 octets by XOR-ing groups of the device_id bytes.
    // This loop ensures each of the five octets gets contributions from the 16 bytes.
    for (int i = 0; i < 5; i++) {
        uint8_t xor_val = 0;
        for (int j = i; j < 16; j += 5) {
            xor_val ^= device_id[j];
        }
        mac_buffer[i + 1] = xor_val;
    }
}

