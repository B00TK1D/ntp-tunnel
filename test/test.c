#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main() {
    printf("Localhost: %d\n", inet_addr("0.0.0.0"));
    return 0;
}