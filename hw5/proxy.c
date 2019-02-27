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
#include "dynamicarray.h"

#define BUFFER_SIZE 1024

forbidden_sites_t * sites;

void checkParam(int argc, char* argv[]);
void proxyError(int code, const char * msg, char * httpversion, char * write_buf, int * buffer_size);
void response403(char * httpversion, char * write_buf, int * buffer_size);
void response501(char * httpversion, char * write_buf, int * buffer_size);
void response405(char * httpversion, char * write_buf, int * buffer_size);
void response400(char * httpversion, char * write_buf, int * buffer_size);

int main(int argc, char *argv[])
{
    checkParam(argc, argv);

    FILE * fp;
    char url[128];
    if((fp = fopen(argv[2], "r+")) == NULL) {
        failHandler("open file error!");
    }
    sites = initSitesArray();
    while (!feof(fp) && !ferror(fp)) {
        fscanf(fp, "%s", url);
        addSiteItem(sites, url);
    }
    fclose(fp);

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
    
        // process request 
        int ret = 0;

        char method[20];
        char request[100];
        char version[20];
        sscanf(browser_buf, "%s %s %s", method, request, version);
        
        if(strcmp(method, "GET") == 0 || strcmp(method, "HEAD") == 0) {

        }else{
            char err[1024];
            int i = 1024;
            response501(version, err, &i);
            write(server_sock, err, BUFFER_SIZE);
            close(server_sock);
            continue;
        }

        char title[30];
        char host[1024];

        char log3[200];
        while(strlen(browser_buf) > ret){
            if(browser_buf[ret] == '\r') {
                strncpy(log3, browser_buf, ret);
                break;
            }
            ret++;
        }
        printf("~~~~~~ reqeust: %s\n",log3);

        ret = 0;

        while(strlen(browser_buf) > ret){
            if(browser_buf[ret] == '\r') {
                sscanf(browser_buf+ret, "%s%s", title, host);
                if(strcmp(title, "Host:") == 0){
                    break;
                }
            }
            ret++;
        }

        printf("!!!!%s\n",browser_buf);
        //filter
        int forbidflag = 0;
        for(int i = 0; i < sites->occupied; i++) {
            if(strcmp(host, getSiteItem(sites, i)) == 0) {
                forbidflag = 1;
                break;
            }
        }
        if (forbidflag) {
            char err[1024];
            int i = 1024;
            response403(version, err, &i);
            write(server_sock, err, BUFFER_SIZE);
            close(server_sock);
            continue;
        }

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

        // real forward
        struct hostent *hp;
        hp = gethostbyname(host);
        if(hp == NULL) {
            char err[1024];
            int i = 1024;
            response400(version, err, &i);
            write(server_sock, err, BUFFER_SIZE);
            close(server_sock);
            continue;
        }
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
            printf("!!!!%s\n",host_buffer);

            if (flag) {

                // logging time 
                time_t t = time(0);   
                char log1[25];   
                strftime(log1, 25, "%Y-%m-%dT%T%Z", localtime(&t));

                flag = 0;
                sscanf(host_buffer, "%s %s %s", protocolversion, responecode, status);
                ret = 0;
                while(strlen(host_buffer) > ret){
                    if(host_buffer[ret] == '\r') {
                        sscanf(host_buffer+ret, "%s%s", title, lengthchar);
                        if(strcmp(title, "Content-Length:") == 0){
                            length = atol(lengthchar);
                            break;
                        }
                    }
                    ret++;
                }
                printf("~~~~~~time %s\n", log1);
                printf("~~~~~~responsecode %s\n", responecode);
                printf("~~~~~~length %ld\n", length);

                if((fp = fopen("access.log", "a")) == NULL) {
                    failHandler("open file error!");
                }
                fprintf(fp, "%s %s \"%s\" %s %ld\n", log1, log2, log3,responecode, length);
                fclose(fp);
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

void response403(char * httpversion, char * write_buf, int * buffer_size) {
    proxyError(403, "Forbidden", httpversion, write_buf, buffer_size);
    printf("%s\n",write_buf);
}

void response501(char * httpversion, char * write_buf, int * buffer_size) {
    proxyError(501, "Not Implemented", httpversion, write_buf, buffer_size);
    printf("%s\n",write_buf);
}

void response405(char * httpversion, char * write_buf, int * buffer_size) {
    proxyError(501, "Not Implemented", httpversion, write_buf, buffer_size);
    printf("%s\n",write_buf);
}

void response400(char * httpversion, char * write_buf, int * buffer_size) {
    proxyError(400, "Bad Request", httpversion, write_buf, buffer_size);
    printf("%s\n",write_buf);
}

void proxyError(int code, const char * msg, char * httpversion, char * write_buf, int * buffer_size) {
    bzero(write_buf, *buffer_size);
    time_t t = time(0);   
    char date[50];
    strftime(date, 50, "%a, %d %b %Y %T %Z", localtime(&t));
    sprintf(write_buf, "%s %d %s\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\nDate: %s\r\n\r\n<!DOCTYPE html>\n<html>\n<title>%d %s</title>\n<p>%d %s</p>\n</html>\n", httpversion, code, msg, date, code, msg, code, msg);
    *buffer_size = strlen(write_buf);
}
