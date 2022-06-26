#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct Packet {
    int header_size;
    int content_size;
    int footer_size;
    char* header;
    char* content;
    char* footer;
} Packet;

typedef struct PacketType {
    int header_size;
    int content_size;
    int footer_size;
    char* header;
    char* footer;
} PacketType;

Packet* packet_init(PacketType type) {
    Packet* packet = malloc(sizeof(Packet));
    packet->header = (char*) malloc(type.header_size);
    packet->content = (char*) malloc(type.content_size);
    packet->footer = (char*) malloc(type.footer_size);
    packet->header = type.header;
    packet->footer = type.footer;
    return packet;
}

#define NTP_HEADER_SIZE 16
#define NTP_CONTENT_SIZE 32
#define NTP_FOOTER_SIZE 0
#define NTP_HEADER {0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
#define NTP_FOOTER NULL
Packet* ntp_packet_init() {
    PacketType ntp_type;
    ntp_type.header_size = NTP_HEADER_SIZE;
    ntp_type.content_size = NTP_CONTENT_SIZE;
    ntp_type.footer_size = NTP_FOOTER_SIZE;

    


    #if NTP_HEADER_SIZE > 0
        char header[NTP_HEADER_SIZE] = NTP_HEADER;
        ntp_type.header = (char*) malloc(NTP_HEADER_SIZE);
        memcpy(ntp_type.header, header, NTP_HEADER_SIZE);
    #else
        ntp_type.header = NULL;
    #endif

    #if NTP_FOOTER_SIZE > 0
        char footer[NTP_FOOTER_SIZE] = NTP_FOOTER;
        ntp_type.footer = (char*) malloc(NTP_FOOTER_SIZE);
        memcpy(ntp_type.header, footer, NTP_FOOTER_SIZE);
    #else
        ntp_type.footer = NULL;
    #endif

    

    return packet_init(ntp_type);
}

Packet* ntp_packet_content(Packet* packet, char* content) {
    if (sizeof(content) >= NTP_CONTENT_SIZE) {
         memcpy(packet->content, content, NTP_CONTENT_SIZE);
    } else {
        int i = 0;
        while (i < sizeof(content)) {
            packet->content[i] = content[i];
            i++;
        }
        while (i < NTP_CONTENT_SIZE) {
            packet->content[i] = 0;
            i++;
        }
    }
    return packet;
}




int main() {
    Packet* packet = ntp_packet_init();
    packet = ntp_packet_content(packet, "Hello World");
    // print each byte of the packet
    for (int i = 0; i < sizeof(packet->header); i++) {
        printf("%02x\n", packet->header[i]);
    }
    for (int i = 0; i < sizeof(packet->content); i++) {
        printf("%02x\n", packet->content[i]);
    }
    for (int i = 0; i < sizeof(packet->footer); i++) {
        printf("%02x\n", packet->footer[i]);
    }
}