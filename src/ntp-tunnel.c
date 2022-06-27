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
    #define DEFAULT_KEY {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
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
    #define COMMAND_LENGTH 1024
    char* command = malloc(sizeof(char) * COMMAND_LENGTH);
    #ifdef OBFUSCATE_ENABLED
        while ((gopt = getopt(argc, argv, "e:loh")) != -1) {
    #else
        while ((gopt = getopt(argc, argv, "elh")) != -1) {
    #endif
        switch (gopt) {
            case 'e':
                opt |= ShellOption;
                strncpy(command, optarg, COMMAND_LENGTH);
                break;
            case 'l':
                opt |= ListenOption;
                break;
            case 'h':
                #ifdef ERROR_ENABLED
                    printf("\nNTP Tunnel - A simple NTP data tunnel client/server\n\n");
                    #ifdef OBFUSTATE_ENABLED
                        printf("Usage: %s [-l] [-s] [-o] address\n", argv[0]);
                    #else
                        printf("Usage: %s [-l] [-s] [-o] address\n", argv[0]);
                    #endif
                    printf("\t-l\tListen on the specified address (for use on the server side)\n");
                    printf("\t-e command\tExecute specified command in interactive mode\n");
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
                        printf("Usage: %s [-l] [-o] [-e command] address\n", argv[0]);
                    #endif
                    exit(EXIT_FAILURE);
            #endif
        }
    }




    fd_set rfds;
    struct timeval tv = {5, 0};
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
    int server_inet;
    if (argv[optind] == NULL) {
        if (opt & ListenOption) {
            server_inet = 0;
        } else {
            #ifdef ERROR_ENABLED
                fprintf(stderr, "Error: no address specified\n");
            #endif
            return -1;
        }
    } else {
        server_inet = inet_addr(argv[optind]);
    }
    server_addr.sin_addr.s_addr = server_inet;

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

    
    #define NUM_PIPES          2
 
    #define PARENT_WRITE_PIPE  0
    #define PARENT_READ_PIPE   1
    
    int pipes[NUM_PIPES][2];
    
    /* always in a pipe[], pipe[0] is for read and 
    pipe[1] is for write */
    #define READ_FD  0
    #define WRITE_FD 1
    
    #define PARENT_READ_FD  ( pipes[PARENT_READ_PIPE][READ_FD]   )
    #define PARENT_WRITE_FD ( pipes[PARENT_WRITE_PIPE][WRITE_FD] )
    
    #define CHILD_READ_FD   ( pipes[PARENT_WRITE_PIPE][READ_FD]  )
    #define CHILD_WRITE_FD  ( pipes[PARENT_READ_PIPE][WRITE_FD]  )

    pipe(pipes[PARENT_READ_PIPE]);
    pipe(pipes[PARENT_WRITE_PIPE]);

    if (opt & ShellOption && fork()) {
        dup2(CHILD_READ_FD, STDIN_FILENO);
        dup2(CHILD_WRITE_FD, STDOUT_FILENO);
        dup2(CHILD_WRITE_FD, STDERR_FILENO);

        close(CHILD_READ_FD);
        close(CHILD_WRITE_FD);
        close(PARENT_READ_FD);
        close(PARENT_WRITE_FD);

        execve(command, NULL, NULL);
    } else {

        if (opt & ShellOption) {
            close(CHILD_READ_FD);
            close(CHILD_WRITE_FD);
        } else {
            PARENT_READ_FD = STDIN_FILENO;
            PARENT_WRITE_FD = STDOUT_FILENO;
        }

        int max_fd;

        while (1) {
            FD_ZERO(&rfds);
            FD_SET(PARENT_READ_FD, &rfds);
            FD_SET(sockfd, &rfds);
            max_fd = sockfd > PARENT_READ_FD ? sockfd : PARENT_READ_FD;
            tv = (struct timeval) {5, 0};
            if(select(max_fd + 1, &rfds, NULL, NULL, &tv)) {
                if (FD_ISSET(PARENT_READ_FD, &rfds)) {
                    memset(packet->content, 0, packet->content_size);
                    read(PARENT_READ_FD, packet->content, packet->content_size);

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
                            write(PARENT_WRITE_FD, packet->content, packet->content_size);
                            fflush(stdout);
                        }
                    } else {
                        if (strcmp(packet->content, NTP_DEFAULT_CONTENT)) {
                            write(PARENT_WRITE_FD, packet->content, packet->content_size);
                            fflush(stdout);
                        }
                    }
                    FD_SET(PARENT_READ_FD, &rfds);
                }
            }/* else {            
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
                fresh_server_addr.sin_addr.s_addr = server_inet;

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
            }*/
        }   


        close(sockfd);
        packet_destroy(packet);
    }
    
    
    exit(EXIT_SUCCESS);
}