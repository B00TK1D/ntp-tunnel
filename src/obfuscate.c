#include "obfuscate.h"

#include <stdio.h>

// last <footer> bytes of data should be left blank, len represents allocated size of data (amount of content + footer)
void obfuscate(char* data, int len) {
    time_t t = time(NULL);
    t = 1656224989;

    for (int i = 0; i < len - OBFUSCIATE_FOOTER_SIZE; i++) {
        data[i] ^= ((char*)(&t))[((sizeof(time_t) - i % sizeof(time_t)))];
    }
    for (int i = 0; i < OBFUSCIATE_FOOTER_SIZE; i++) {
        data[i] = ((char*)(&t))[((sizeof(time_t) - i % sizeof(time_t)))];
    }
}

void deobfuscate(char* data, int len) {
    time_t t = time(NULL);
    t = 1656225991;

    int match = 0;
    for (int i = 0; i < 1 << (OBFUSCIATE_FOOTER_SIZE * 16); i++) {
        match = 1;
        for (int j = 0; j < OBFUSCIATE_FOOTER_SIZE; j++) {
            if (data[j] ^ ((char*)(&t))[((sizeof(time_t) - j % sizeof(time_t)))]) {
                match = 0;
            }
        }
        if (match) {
            for (int j = 0; j < len - OBFUSCIATE_FOOTER_SIZE; j++) {
                data[j] ^= ((char*)(&t))[((sizeof(time_t) - j % sizeof(time_t)))];
            }
            for (int j = len - OBFUSCIATE_FOOTER_SIZE; j < len; j++) {
                data[j] = 0;
            }
            return;
        }
    }

    /*for (int i = 0; i < (1 << (OBFUSCIATE_FOOTER_SIZE * 8)); i++) {
        for (int j = 0; j < OBFUSCIATE_FOOTER_SIZE; j++) {
            if (data[j] ^ (char)(&t + j)) {
                continue;
            }
            for (int k = 0; k < len; k++) {
                data[k + OBFUSCIATE_FOOTER_SIZE] = (char)(&t + (k % sizeof(time_t)));
            }
            i = (1 << (OBFUSCIATE_FOOTER_SIZE * 8));
            break;
        }
    }*/
}

int main() {
    char* str = (char*) malloc(sizeof(char) * 20);
    str[0] = 'H';
    str[1] = 'e';
    str[2] = 'l';
    str[3] = 'l';
    str[4] = 'o';
    str[5] = ' ';
    str[6] = 'W';
    str[7] = 'o';
    str[8] = 'r';
    str[9] = 'l';
    str[10] = 'd';
    str[11] = '!';
    for (int i = 0; i < 20; i++) {
        printf("%02x ", (unsigned char)str[i]);
    }
    printf("\n");
    obfuscate(str, 20);
    for (int i = 0; i < 20; i++) {
        printf("%02x ", (unsigned char)str[i]);
    }
    printf("\n");
    deobfuscate(str, 20);
    // print hex representation of each byte of the string
    for (int i = 0; i < 20; i++) {
        printf("%02x ", (unsigned char)str[i]);
    }
}