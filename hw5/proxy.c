#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "util.h"

#define BUFFER_SIZE 1024

void checkParam(int argc, char* argv[]);

int main(int argc, char *argv[])
{
    checkParam(argc, argv);

    int port = atoi(argv[1]);

    int proxy_sock;
    if((proxy_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        failHandler("create socket error!");
    }
    int optval=1;
    if (setsockopt(proxy_sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int)) < 0) {
        failHandler("create socket error!");
    }
    struct sockaddr_in addr;
    bzero((char *) &addr, sizeof(addr));
    addr.sin_family = AF_INET; 
    addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    addr.sin_port = htons(port); 
    if (bind(proxy_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        failHandler("bind socket error!");
    }

    while(1) {
        if(listen(proxy_sock, 1) < 0){
            close(proxy_sock);
            failHandler("listen socket error!");
        }
        int addr_len = sizeof(addr);
        int server_sock;
        if((server_sock = accept(proxy_sock, (struct sockaddr*)&addr, (socklen_t *) &addr_len)) < 0) {
            continue;
        }
        char browser_buf[BUFFER_SIZE];
        bzero(browser_buf, BUFFER_SIZE);
        read(server_sock, browser_buf, BUFFER_SIZE);
        
        int host_sock;
        if((host_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            failHandler("create socket error!");
        }
        
        int ret = 0;

        char method[20];
        char request[100];
        char version[20];
        sscanf(browser_buf, "%s %s %s", method, request, version);
        while(strlen(browser_buf) > ret){
            if(browser_buf[ret] != '\r') {
                ret++;
            }else {
                break;
            }
        }

        char title[30];
        char host[1024];
        sscanf(browser_buf + ret, "%s %s", title, host);
        while(strlen(browser_buf) > ret){
            if(browser_buf[ret] != '\r') {
                ret++;
            }else {
                break;
            }
        };

        struct hostent *hp;
        hp = gethostbyname(host);
        struct sockaddr_in hostaddr;
        bzero((char *)&hostaddr, sizeof(hostaddr));
        hostaddr.sin_family = AF_INET; 
        bcopy((char *)hp->h_addr_list[0],(char *)&hostaddr.sin_addr.s_addr, hp->h_length);
        hostaddr.sin_port = htons(80);

        addr_len = sizeof(hostaddr);
        if(connect(host_sock, (struct sockaddr*)&hostaddr, addr_len) < 0) {
            failHandler("connect host socket error!");
        }
        write(host_sock, browser_buf, BUFFER_SIZE);

        char host_buffer[BUFFER_SIZE];
        bzero(host_buffer, BUFFER_SIZE);
        while(read(host_sock, host_buffer, BUFFER_SIZE) > 0) {
            write(server_sock, host_buffer, BUFFER_SIZE);
        }
        close(host_sock);
        close(server_sock);
    }

    return 0;
}

void checkParam(int argc, char* argv[]) {

	if(argc != 3) {
        failHandler("Please use client by correct input! usage: ./proxy <listen-port> <forbidden-sites-file>");
    }

    if(!isNumber(argv[1])) {
        failHandler("Please input correct port number!");
    }

    if(!hasFile(argv[2])) {
        failHandler("forbidden-sites-file does not exist!");
    }
}