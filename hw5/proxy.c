#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "util.h"
#include <time.h>

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
        close(proxy_sock);
        failHandler("set socket error!");
    }
    struct sockaddr_in proxy_addr;
    bzero((char *) &proxy_addr, sizeof(proxy_addr));
    proxy_addr.sin_family = AF_INET; 
    proxy_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    proxy_addr.sin_port = htons(port); 
    if (bind(proxy_sock, (struct sockaddr*)&proxy_addr, sizeof(proxy_addr)) < 0) {
        close(proxy_sock);
        failHandler("bind socket error!");
    }

    int addr_len;

    while(1) {
        if(listen(proxy_sock, 1) < 0){
            close(proxy_sock);
            failHandler("listen socket error!");
        }

        // accept
        addr_len = sizeof(proxy_addr);
        int server_sock;
        if((server_sock = accept(proxy_sock, (struct sockaddr*)&proxy_addr, (socklen_t *) &addr_len)) < 0) {
            continue;
        }
        

        // get request from client
        char browser_buf[BUFFER_SIZE];
        bzero(browser_buf, BUFFER_SIZE);
        int len = read(server_sock, browser_buf, BUFFER_SIZE);
    


        // logging time 
        time_t t = time(0);   
	    char log1[25];   
	    strftime(log1, 25, "%Y-%m-%dT%T%Z", localtime(&t));



        // process request 
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

        char log3[200];
        strncpy(log3, browser_buf, ret);

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



        // add forward message 
        char proxyHostName[255];
        gethostname(proxyHostName, 255);
        struct hostent * proxyHost;
        proxyHost=gethostbyname(proxyHostName);
        char proxy_ip[100];
        inet_ntop(AF_INET,proxyHost->h_addr_list[0] ,proxy_ip, INET_ADDRSTRLEN);


        struct sockaddr_in peer;
        int peer_len = sizeof(peer);
        if (getpeername(server_sock, (struct sockaddr *)&peer, (socklen_t *)&peer_len) == -1) {
            failHandler("create socket error!");
        }
        char log2[100];
        inet_ntop(AF_INET, &peer.sin_addr ,log2, INET_ADDRSTRLEN);

        char forwardstr[128];
        sprintf(forwardstr, "Forwarded: for=%s; proto=http; by=%s\r\n\r\n", log2, proxy_ip);

        strcpy(&browser_buf[len-2], forwardstr);
        // printf("\n\n%s\n", browser_buf);
        



        // real forward
        struct hostent *hp;
        hp = gethostbyname(host);
        struct sockaddr_in hostaddr;
        bzero((char *)&hostaddr, sizeof(hostaddr));
        hostaddr.sin_family = AF_INET; 
        bcopy((char *)hp->h_addr_list[0],(char *)&hostaddr.sin_addr.s_addr, hp->h_length);
        hostaddr.sin_port = htons(80);

        int host_sock;
        if((host_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            close(proxy_sock);
            failHandler("create socket error!");
        }

        addr_len = sizeof(hostaddr);
        if(connect(host_sock, (struct sockaddr*)&hostaddr, addr_len) < 0) {
            failHandler("connect host socket error!");
        }
        write(host_sock, browser_buf, BUFFER_SIZE);

        //get resoponse and forward to client
        char host_buffer[BUFFER_SIZE];
        bzero(host_buffer, BUFFER_SIZE);
        int flag = 1;
        char protocolversion[20];
        char responecode[100];
        char status[20];
        int l;
        char lengthchar[64];
        long length = 0;
        while((l = read(host_sock, host_buffer, BUFFER_SIZE)) > 0) {
            host_buffer[l] = 0x00;
            if (flag) {
                printf("@@@@@@@@@@%s\n", host_buffer);
                flag = 0;
                sscanf(host_buffer, "%s %s %s", protocolversion, responecode, status);
                printf("~~~~~~~~~~~~~~~~~~~~~~~~~\n");
                ret = 0;
                while(strlen(host_buffer) > ret){
                    if(host_buffer[ret] != '\r') {
                        // printf("%c",host_buffer[ret]);
                        ret++;
                    }else {
                        ret++;
                        // printf("~~~~~~~\r");
                        sscanf(host_buffer+ret, "%s%s", title, lengthchar);
                        printf("~~~%s\n", title);
                        if(strcmp(title, "Content-Length:") == 0){
                            printf("!!!!!!\n");
                            length = atol(lengthchar);
                            break;
                        }else{
                            ret++;
                        }
                    }
                }

                printf("content length : %ld\n", length);
            }

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