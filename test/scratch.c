void server() {
    fd_set rfds;
    struct timeval tv = {5, 0};
    Packet* packet = packet_init(NTP_PACKET);

    int sockfd = NTP_SOCK;
    struct sockaddr_in serv_addr = NTP_ADDR;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    while (1) {
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        if(select(1, &rfds, NULL, NULL, &tv)) {
            memset(packet->content, 0, packet->content_size);
            read(STDIN_FILENO, packet->content, packet->content_size);
            printf("%s", packet->content);
        }
    }
}