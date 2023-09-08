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
                          unsigned long src_addr, unsigned long dest_addr, unsigned short flags)
{
    
    size_t messageLen = strlen(message);
	unsigned int numPackets = (messageLen + max_payload - 1) / max_payload;
	unsigned int headerSize = 24;

	if (numPackets > packets_len) {
		numPackets = packets_len;
		messageLen = numPackets * max_payload;
	}
	unsigned int payloadIdx = 0;
	for (unsigned int i = 0; i < numPackets; i++) {
		unsigned int payloadSize = max_payload;
		if (payloadIdx + payloadSize > messageLen) {
			payloadSize = messageLen - payloadIdx;
		}
		packets[i] = malloc(headerSize + payloadSize);
		if (packets[i] == NULL) {
			return i;
		}
        //srcAdd
        unsigned long tempSrcAddr = src_addr;
        packets[i][4]=tempSrcAddr&0xff;
        tempSrcAddr>>=8;
        packets[i][3]=tempSrcAddr&0xff;
        tempSrcAddr>>=8;
        packets[i][2]=tempSrcAddr&0xff;
        tempSrcAddr>>=8;
        packets[i][1]=tempSrcAddr&0xff;
        tempSrcAddr>>=8;
        packets[i][0]=tempSrcAddr&0xff;
        tempSrcAddr>>=8;
    
        //destAdd
        unsigned long tempDestAddr = dest_addr;
        packets[i][9]=tempDestAddr&0xff;
        tempDestAddr>>=8;
        packets[i][8]=tempDestAddr&0xff;
        tempDestAddr>>=8;
        packets[i][7]=tempDestAddr&0xff;
        tempDestAddr>>=8;
        packets[i][6]=tempDestAddr&0xff;
        tempDestAddr>>=8;
        packets[i][5]=tempDestAddr&0xff;
        tempDestAddr>>=8;

        //srcPort
        packets[i][10]=32;

        //destPort
        packets[i][11]=64;

        //fragOff
        unsigned long tempIdx= payloadIdx;
        packets[i][14]=tempIdx&0xff;
        tempIdx>>=8;
        packets[i][13]=tempIdx&0xff;
        tempIdx>>=8;
        packets[i][12]=tempIdx&0xff;;

        //flag
        unsigned int tempFlags = flags;
        packets[i][16]=tempFlags&0xff;
        tempFlags>>=8;
        packets[i][15]=tempFlags&0xff;

        //totalLength
        unsigned long tempSize = payloadSize + headerSize;
        packets[i][19]=tempSize&0xff;
        tempSize>>=8;
        packets[i][18]=tempSize&0xff;
        tempSize>>=8;
        packets[i][17]=tempSize&0xff;
        tempSize>>=8;

        //checksum
        unsigned int tempChecksum = checksum_sf(packets[i]);
        packets[i][23]=tempChecksum&0xff;
        tempChecksum>>=8;
        packets[i][22]=tempChecksum&0xff;
        tempChecksum>>=8;
        packets[i][21]=tempChecksum&0xff;
        tempChecksum>>=8;
        packets[i][20]=tempChecksum&0xff;
        tempChecksum>>=8;

		
        for(unsigned int j =0;j<payloadSize;j++){
            packets[i][headerSize + j] = message[payloadIdx+j];
        }
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