#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "obfuscate.h"
#include "packet.h"
#include "ntp.h"


enum Option
{
    ListenOption = 1,
    ObfusticateOption = 2,
    ShellOption = 4
} Option;

int main(int argc, char* argv[]) {    
    enum Option opt = 0;
    int gopt = 0;
    while ((gopt = getopt(argc, argv, "los")) != -1) {
        switch (gopt) {
        case 'l': opt |= ListenOption; break;
        case 'o': opt |= ObfusticateOption; break;
        case 's': opt |= ShellOption; break;
        default:
            fprintf(stderr, "Usage: %s [-los] [address]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }


    fd_set rfds;
    struct timeval tv = {10, 0};
    Packet* packet = packet_init(NTP_PACKET);
    char buffer[packet->size];

    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in server_addr, client_addr;
    socklen_t sock_struct_length = sizeof(server_addr);

    if(sockfd < 0){
        fprintf(stderr, "Error: couldn't create socket\n");
        return -1;
    }   

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(123);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (opt & ListenOption) {
        if(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
            fprintf(stderr, "Error: couldn't bind to the port\n");
            return -1;
        }
    }

    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);
    FD_SET(sockfd, &rfds);

    while (1) {
        if(select(sockfd + 1, &rfds, NULL, NULL, &tv)) {
            if (FD_ISSET(STDIN_FILENO, &rfds)) {
                memset(packet->content, 0, packet->content_size);
                if (opt & ObfusticateOption) {
                    read(STDIN_FILENO, packet->content, packet->content_size - OBFUSCIATE_FOOTER_SIZE);
                } else {
                    read(STDIN_FILENO, packet->content, packet->content_size);
                }
                if (opt & ListenOption) {
                    sendto(sockfd, packet_stream(packet), packet->size, 0, (struct sockaddr*)&client_addr, sock_struct_length);
                } else {
                    sendto(sockfd, packet_stream(packet), packet->size, 0, (struct sockaddr*)&server_addr, sock_struct_length);
                }
                FD_SET(sockfd, &rfds);
            } else {
                memset(packet->content, 0, packet->content_size);
                if (opt & ListenOption) {
                    recvfrom(sockfd, buffer, packet->size, 0, (struct sockaddr*)&client_addr, &sock_struct_length);
                } else {
                    recvfrom(sockfd, buffer, packet->size, 0, (struct sockaddr*)&server_addr, &sock_struct_length);
                }
                packet_parse(packet, buffer);

                if (opt & ObfusticateOption) {
                    deobfuscate(packet->content, packet->content_size);
                }

                if (strcmp(packet->content, NTP_DEFAULT_CONTENT)) {
                    write(STDOUT_FILENO, packet->content, packet->content_size);
                    fflush(stdout);
                }
                FD_SET(STDIN_FILENO, &rfds);
            }
        } else {            
            close(sockfd);
            sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

            if(sockfd < 0){
                fprintf(stderr, "Error: couldn't create socket\n");
                return -1;
            }

            struct sockaddr_in fresh_server_addr, fresh_client_addr;

            fresh_server_addr.sin_family = AF_INET;
            fresh_server_addr.sin_port = htons(123);
            fresh_server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

            server_addr = fresh_server_addr;
            client_addr = fresh_client_addr;

            if (opt & ListenOption) {
                if(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
                    fprintf(stderr, "Error: couldn't bind to the port\n");
                    return -1;
                }
            } else {
                packet->content = NTP_DEFAULT_CONTENT;
                sendto(sockfd, packet_stream(packet), packet->size, 0, (struct sockaddr*)&server_addr, sock_struct_length);
            }
            
            FD_ZERO(&rfds);
            FD_SET(STDIN_FILENO, &rfds);
            FD_SET(sockfd, &rfds);
        }
    }

    close(sockfd);
    packet_destroy(packet);
    
    exit(EXIT_SUCCESS);
}