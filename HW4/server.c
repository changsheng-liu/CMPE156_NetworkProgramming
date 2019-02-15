#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

#define BUFF_LEN 1024

int main(int argc, char* argv[])
{
    int server_fd;
    struct sockaddr_in ser_addr,clent_addr; 

    if((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        failHandler("create socket error!");
    }

    bzero(&ser_addr, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ser_addr.sin_port = htons(atoi(argv[1]));

    if((bind(server_fd, (struct sockaddr*)&ser_addr, sizeof(ser_addr))) < 0) {
        failHandler("bind socket error!");
    }

    char buf[BUFF_LEN];  
    socklen_t len;
    struct sockaddr_in ;
    for( ; ; ) {
        bzero(buf, BUFF_LEN);
        len = sizeof(clent_addr);
        if(recvfrom(server_fd, buf, BUFF_LEN, 0, (struct sockaddr*)&clent_addr, &len) < 0) {
            failHandler("receive error!");
        }
        fputs(buf, stdout);
        sendto(server_fd, buf, BUFF_LEN, 0, (struct sockaddr*)&clent_addr, len);
    }

    close(server_fd);
    return 0;
}