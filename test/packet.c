#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


typedef struct Packet {
    int header_size;
    int content_size;
    int footer_size;
    char* header;
    char* content;
    char* footer;
} Packet;

#define NTP_CONTENT_SIZE 32
#define NTP_HEADER (char[]) {0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
#define NTP_FOOTER (char[]) {}
#define NTP_PACKET NTP_CONTENT_SIZE, sizeof(NTP_HEADER), NTP_HEADER, sizeof(NTP_FOOTER), NTP_FOOTER


Packet* packet_init(int content_size, int header_size, char header[], int footer_size, char footer[]) {
    Packet* packet = malloc(sizeof(Packet));
    packet->header_size = header_size;
    packet->content_size = content_size;
    packet->footer_size = footer_size;
    packet->header = (char*) malloc(header_size);
    packet->content = (char*) calloc(content_size, sizeof(char));
    packet->footer = (char*) malloc(footer_size);
    memcpy(packet->header, NTP_HEADER, header_size);
    memcpy(packet->header, NTP_FOOTER, footer_size);
    return packet;
}

char* packet_stream(Packet* packet, int* len) {
    *len = packet->header_size + packet->content_size + packet->footer_size;
    char* stream = (char*) malloc(*len);
    memcpy(stream, packet->header, packet->header_size);
    memcpy(stream + packet->header_size, packet->content, packet->content_size);
    memcpy(stream + packet->header_size + packet->content_size, packet->footer, packet->footer_size);
    return stream;
}

int packet_parse(Packet* packet, char* stream) {
    memcpy(packet->header, stream, packet->header_size);
    memcpy(packet->content, stream + packet->header_size, packet->content_size);
    memcpy(packet->footer, stream + packet->header_size + packet->content_size, packet->footer_size);
}


int main() {
    Packet* packet = packet_init(NTP_PACKET);
    packet->content[0] = 'h';
    packet->content[1] = 'e';
    packet->content[2] = 'l';
    packet->content[3] = 'l';
    packet->content[4] = 'o';

    int len;
    char* stream = packet_stream(packet, &len);
    //write(1, packet_stream(packet, &len), len);

    stream[18] = 'w';

    packet_parse(packet, stream);
    write(1, packet->content, packet->content_size);
}