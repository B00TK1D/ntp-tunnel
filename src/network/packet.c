#include "packet.h"


Packet* packet_init(int content_size, int header_size, char header[], int footer_size, char footer[]) {
    Packet* packet = malloc(sizeof(Packet));
    packet->size = header_size + content_size + footer_size;
    packet->header_size = header_size;
    packet->content_size = content_size;
    packet->footer_size = footer_size;
    packet->header = (char*) malloc(header_size);
    packet->content = (char*) calloc(content_size, sizeof(char));
    packet->footer = (char*) malloc(footer_size);
    memcpy(packet->header, header, header_size);
    memcpy(packet->header, footer, footer_size);
    return packet;
}

void packet_destroy(Packet* packet) {
    free(packet->header);
    free(packet->content);
    free(packet->footer);
    free(packet);
}

char* packet_stream(Packet* packet, char* stream) {
    memcpy(stream, packet->header, packet->header_size);
    memcpy(stream + packet->header_size, packet->content, packet->content_size);
    memcpy(stream + packet->header_size + packet->content_size, packet->footer, packet->footer_size);
    return stream;
}

void packet_parse(Packet* packet, char* stream) {
    memcpy(packet->header, stream, packet->header_size);
    memcpy(packet->content, stream + packet->header_size, packet->content_size);
    memcpy(packet->footer, stream + packet->header_size + packet->content_size, packet->footer_size);
}