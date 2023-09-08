#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>

#include "wolfpack.h"

void print_packet_sf(const unsigned char *packet)
{
    for (int i = 0; i < 5; i++)
    {
        printf("%02x", packet[i]);
    }
    printf("\n");
    for (int i = 5; i < 10; i++)
    {
        printf("%02x", packet[i]);
    }
    printf("\n");
    printf("%02x\n", packet[10]);

    printf("%02x\n", packet[11]);

    for (int i = 12; i < 15; i++)
    {
        printf("%02x", packet[i]);
    }
    printf("\n");

    printf("%04x\n", (packet[15] << 8) | packet[16]);

    for (int i = 17; i < 20; i++)
    {
        printf("%02x", packet[i]);
    }
    printf("\n");

    for (int i = 20; i < 24; i++)
    {
        printf("%02x", packet[i]);
    }
    printf("\n");

    int payload_length = ((packet[17] << 16) | (packet[18] << 8) | packet[19]) - 24;
    for (int i = 0; i < payload_length; i++)
    {
        printf("%c", (char)packet[i + 24]);
    }
    printf("\n");
}


unsigned int packetize_sf(const char *message, unsigned char *packets[], unsigned int packets_len, unsigned int max_payload,
                          unsigned long src_addr, unsigned long dest_addr, unsigned short flags) {
    
    size_t messageLen = strlen(message);
    unsigned int numPackets = (messageLen + max_payload - 1) / max_payload;
    unsigned int headerSize = 24;

    if (numPackets > packets_len) {
        numPackets = packets_len;
        messageLen = numPackets * max_payload;
    }

    unsigned int payloadIdx = 0;
    for (unsigned int i = 0; i < numPackets; i++) {
        unsigned int payloadSize = (payloadIdx + max_payload > messageLen) ? (messageLen - payloadIdx) : max_payload;
        
        packets[i] = malloc(headerSize + payloadSize);
        if (!packets[i]) return i;

        unsigned long tempSrcAddr = src_addr;
        for (int j = 4; j >= 0; j--) {
            packets[i][j] = tempSrcAddr & 0xff;
            tempSrcAddr >>= 8;
        }

        unsigned long tempDestAddr = dest_addr;
        for (int j = 9; j >= 5; j--) {
            packets[i][j] = tempDestAddr & 0xff;
            tempDestAddr >>= 8;
        }

        packets[i][10] = 32; // srcPort
        packets[i][11] = 64; // destPort

        unsigned long tempIdx = payloadIdx;
        for (int j = 14; j >= 12; j--) {
            packets[i][j] = tempIdx & 0xff;
            tempIdx >>= 8;
        }

        unsigned short tempFlags = flags;
        for (int j = 16; j >= 15; j--) {
            packets[i][j] = tempFlags & 0xff;
            tempFlags >>= 8;
        }

        unsigned long totalSize = payloadSize + headerSize;
        for (int j = 19; j >= 17; j--) {
            packets[i][j] = totalSize & 0xff;
            totalSize >>= 8;
        }

        unsigned int tempChecksum = checksum_sf(packets[i]);
        for (int j = 23; j >= 20; j--) {
            packets[i][j] = tempChecksum & 0xff;
            tempChecksum >>= 8;
        }

        memcpy(&packets[i][headerSize], &message[payloadIdx], payloadSize);
        payloadIdx += payloadSize;
    }
    return numPackets;
}


unsigned long long get_field_value(const unsigned char *field, int size) {
    unsigned long long field_value = 0;
    for(int i = 0; i < size; i++) {
        field_value <<= 8;
        field_value |= field[i];
    }
    return field_value;
}

unsigned int checksum_sf(const unsigned char *packet)
{
   unsigned long long check_sum = 0;
   check_sum += get_field_value(packet, 5);
   check_sum += get_field_value(packet + 5, 5);
   check_sum += get_field_value(packet + 10, 1);
   check_sum += get_field_value(packet + 11, 1);
   check_sum += get_field_value(packet + 12, 3);
   check_sum += get_field_value(packet + 15, 2);
   check_sum += get_field_value(packet + 17, 3);
   unsigned long long total_check_sum = check_sum;
   
   total_check_sum %= (1ull << 32) - 1;

   return total_check_sum;
   
}

unsigned int reconstruct_sf(unsigned char *packets[], unsigned int packets_len, char *message, unsigned int message_len)
{
   unsigned int num_reconstruct = 0;
    unsigned int last_written = 0;
    for (unsigned int i = 0; i < packets_len; i++)
    {
        unsigned char *packet = packets[i];
        if (checksum_sf(packet) != get_field_value(packet + 20, 4))
        {
            continue;
        }
        unsigned char *payload = packet + 24;
        unsigned int fragOff = get_field_value(packet + 12, 3);
        if (fragOff > message_len - 1)
        {
            continue;
        }
        int payload_len = get_field_value(packet + 17, 3) - 24;
        for (int j = 0; fragOff + j < message_len - 1 && j < payload_len; j++)
        {
            message[fragOff + j] = payload[j];
            if (fragOff + j > last_written)
            {
                last_written = fragOff + j;
            }
        }
        num_reconstruct++;
    }
    message[last_written + 1] = '\0';
    return num_reconstruct;
}