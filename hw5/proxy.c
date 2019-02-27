#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "util.h"
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include "dynamicarray.h"
#include <pthread.h>

#define BUFFER_SIZE 1024
forbidden_sites_t * sites;

void proxyResponseError(int bad_sock , int code, char * httpversion);

void getlocalIP(char * str) {
    char proxyHostName[255];
    gethostname(proxyHostName, 255);
    struct hostent * proxyHost;
    proxyHost=gethostbyname(proxyHostName);
    inet_ntop(AF_INET,proxyHost->h_addr_list[0] ,str, INET_ADDRSTRLEN);
}

void getclientIP(char * str, int sock) {
    struct sockaddr_in peer;
    int peer_len = sizeof(peer);
    if (getpeername(sock, (struct sockaddr *)&peer, (socklen_t *)&peer_len) == -1) {
        failHandler("create socket error!");
    }
    inet_ntop(AF_INET, &peer.sin_addr ,str, INET_ADDRSTRLEN);
}

void generateLogTime(char * buf) {
    //this function is from https://stackoverflow.com/questions/3673226/how-to-print-time-in-format-2009-08-10-181754-811
    int templen = 20;
    char temp[templen];
	int millisec;
	struct tm* tm_info;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	millisec = lrint(tv.tv_usec/1000.0); 
	if (millisec>=1000) { 
		millisec -=1000;
		tv.tv_sec++;
	}
	tm_info = localtime(&tv.tv_sec);
	strftime(temp, templen, "%Y-%m-%dT%T", tm_info);
    sprintf(buf,"%s.%03dZ", temp, millisec);
}

void remotelog(char * client_ip, char * firstline, char * responsecode, long responselength){
    char date[26];   
    bzero(date, 26);
    generateLogTime(date);
    FILE * fp;
    if((fp = fopen("access.log", "a")) == NULL) {
        failHandler("open file error!");
    }
    fprintf(fp, "%s %s \"%s\" %s %ld\n", date, client_ip, firstline,responsecode, responselength);
    fclose(fp);
}

void thread_proxy(int server_sock) {

        // get request from client
        char browser_buf[BUFFER_SIZE*2];
        bzero(browser_buf, BUFFER_SIZE*2);
        int len = read(server_sock, browser_buf, BUFFER_SIZE*2);
    
        // process request 
        char method[BUFFER_SIZE];
        bzero(method, BUFFER_SIZE);
        char request[BUFFER_SIZE];
        bzero(request, BUFFER_SIZE);
        char version[BUFFER_SIZE];
        bzero(version, BUFFER_SIZE);
        sscanf(browser_buf, "%s %s %s", method, request, version);
        
        if(strcmp(method, "GET") != 0 && strcmp(method, "HEAD") != 0) {
            proxyResponseError(server_sock, 501, version);
            return;
        }

        int ret = 0;
        char firstline[BUFFER_SIZE];
        bzero(firstline, BUFFER_SIZE);
        while(strlen(browser_buf) > ret){
            if(browser_buf[ret] == '\r') {
                strncpy(firstline, browser_buf, ret);
                break;
            }
            ret++;
        }

        ret = 0;
        int checkflag = 0;
        char title[BUFFER_SIZE];
        bzero(title, BUFFER_SIZE);
        char host[BUFFER_SIZE];
        bzero(host, BUFFER_SIZE);
        while(strlen(browser_buf) > ret){
            if(browser_buf[ret] == '\r') {
                sscanf(browser_buf+ret, "%s%s", title, host);
                if(strcmp(title, "Host:") == 0){
                    checkflag = 1;
                    break;
                }
            }
            ret++;
        }

        //filter
        checkflag = 0;
        for(int i = 0; i < sites->occupied; i++) {
            if(strcmp(host, getSiteItem(sites, i)) == 0) {
                checkflag = 1;
                break;
            }
        }
        if (checkflag) {
            proxyResponseError(server_sock, 403, version);
            return;
        }

        //get local ip
        char proxy_ip[BUFFER_SIZE];
        bzero(proxy_ip, BUFFER_SIZE);
        getlocalIP(proxy_ip);

        //get client ip
        char client_ip[BUFFER_SIZE];
        bzero(client_ip, BUFFER_SIZE);
        getclientIP(client_ip, server_sock);
        
        // add forward message 
        char forwardstr[BUFFER_SIZE];
        bzero(forwardstr, BUFFER_SIZE);
        sprintf(forwardstr, "Forwarded: for=%s; proto=http; by=%s\r\n\r\n", client_ip, proxy_ip);
        strcpy(&browser_buf[len-2], forwardstr);

        // real forward
        struct hostent *hp;
        hp = gethostbyname(host);
        if(hp == NULL) {
            proxyResponseError(server_sock, 400, version);
            return;
        }
        struct sockaddr_in hostaddr;
        bzero((char *)&hostaddr, sizeof(hostaddr));
        hostaddr.sin_family = AF_INET; 
        bcopy((char *)hp->h_addr_list[0],(char *)&hostaddr.sin_addr.s_addr, hp->h_length);
        hostaddr.sin_port = htons(80);

        int host_sock;
        if((host_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            failHandler("create socket error!");
        }

        int addr_len2 = sizeof(hostaddr);
        if(connect(host_sock, (struct sockaddr*)&hostaddr, addr_len2) < 0) {
            failHandler("connect host socket error!");
        }
        write(host_sock, browser_buf, BUFFER_SIZE*2);

        //get resoponse and forward to client
        char host_buffer[BUFFER_SIZE];
        bzero(host_buffer, BUFFER_SIZE);
        char responsecode[8];
        bzero(responsecode, 8);
        char status[2];
        bzero(status, 2);
        char lengthchar[64];
        bzero(lengthchar, 64);
       

        long length = 0;
        checkflag = 1;
        while((len = read(host_sock, host_buffer, BUFFER_SIZE)) > 0) {
            
            host_buffer[len] = 0x00;

            if (checkflag) {
                checkflag = 0;
                sscanf(host_buffer, "%s %s %s", version, responsecode, status);
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
                
                remotelog(client_ip,firstline,responsecode,length);
            }

            write(server_sock, host_buffer, BUFFER_SIZE);
        }
        close(host_sock);
        close(server_sock);
}

void proxyResponseError(int bad_sock , int code, char * httpversion) {
    char write_buf[BUFFER_SIZE];
    bzero(write_buf, BUFFER_SIZE);
    time_t t = time(0);   
    char date[50];
    strftime(date, 50, "%a, %d %b %Y %T %Z", localtime(&t));
    char * msg;
    if(code == 403) {
        msg = "Forbidden";
    }else if(code == 501) {
        msg = "Not Implemented";
    }else if(code == 400) {
        msg = "Bad Request";
    }else if(code == 405) {
        msg = "Method Not Allowed";
    }else {
        msg = "Bad Request";
    }
    sprintf(write_buf, "%s %d %s\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\nDate: %s\r\n\r\n<!DOCTYPE html>\n<html>\n<title>%d %s</title>\n<p>%d %s</p>\n</html>\n", httpversion, code, msg, date, code, msg, code, msg);
    write(bad_sock, write_buf, BUFFER_SIZE);
    close(bad_sock);
}

void * thread_handler(void * param) {
    if (pthread_detach(pthread_self()) != 0)
        exit(1);
    int * server_sock_p = (int *)param;
    thread_proxy(*server_sock_p);
    pthread_exit(NULL);
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

    if(listen(proxy_sock, 5) < 0){
        close(proxy_sock);
        failHandler("listen socket error!");
    }

    while(1) {
        // accept
        int addr_len = sizeof(proxy_addr);
        int server_sock;
        if((server_sock = accept(proxy_sock, (struct sockaddr*)&proxy_addr, (socklen_t *) &addr_len)) < 0) {
            continue;
        }
        pthread_t thread;
        // thread_proxy(server_sock);
        if (pthread_create(&thread, NULL, thread_handler, (void *)&server_sock) != 0) {
            failHandler("create thread error!");
        }
    }

    return 0;
}
