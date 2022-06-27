#include <stdlib.h>
#include <string.h>

typedef struct Packet {
    int size;
    int header_size;
    int content_size;
    int footer_size;
    char* header;
    char* content;
    char* footer;
} Packet;

Packet* packet_init(int content_size, int header_size, char header[], int footer_size, char footer[]);

void packet_destroy(Packet* packet);

char* packet_stream(Packet* packet, char* stream);

void packet_parse(Packet* packet, char* stream);