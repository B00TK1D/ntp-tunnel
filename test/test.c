#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>

#define BUF_SIZE 32


void send_packet(int sockfd, char* packet, int packet_size) {
    int n;
    n = write(sockfd, packet, packet_size);
    if (n < 0) {
        perror("ERROR writing to socket");
        exit(EXIT_FAILURE);
    }
}


int main(void) {
    fd_set rfds;
    struct timeval tv, zero;
    int retval;
    char buffer[BUF_SIZE + 1];

    buffer[BUF_SIZE] = '\0';

    /* Watch stdin (fd 0) to see when it has input. */

    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);

    /* Wait up to five seconds. */

    tv.tv_sec = 5;
    tv.tv_usec = 0;

    zero.tv_sec = 0;
    zero.tv_usec = 0;

    while (1) {

        /*
        retval = select(1, &rfds, NULL, NULL, &tv);
        printf("retval: %d\n", retval);
        read(STDIN_FILENO, &buffer[buff_i], 1);
        retval = select(1, &rfds, NULL, NULL, NULL);
        printf("retval: %d\n", retval);
        */
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);

        retval = select(1, &rfds, NULL, NULL, &tv);

        //printf("retval: %d\n", retval);

        if(retval) {

            /*for (int i = 0; i < BUF_SIZE; i++) {
                if (retval) {
                    read(STDIN_FILENO, &(buffer[i]), 1);
                    FD_ZERO(&rfds);
                    FD_SET(STDIN_FILENO, &rfds);
                    retval = select(1, &rfds, NULL, NULL, &zero);
                } else {
                    buffer[i] = '0';
                }
            }*/
            memset(buffer, 0, BUF_SIZE + 1);
            read(STDIN_FILENO, buffer, BUF_SIZE);
            printf("%s", buffer);
        }
    }
    
    exit(EXIT_SUCCESS);
}