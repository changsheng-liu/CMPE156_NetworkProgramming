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
#define REQUERST_SIZE 2048
#define TEMP_SIZE 32
forbidden_sites_t * sites;
pthread_mutex_t filelock = PTHREAD_MUTEX_INITIALIZER;


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
    pthread_mutex_lock(&filelock);
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
    pthread_mutex_unlock(&filelock);
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
   
    struct timeval tv;
    tv.tv_sec = 60;
    tv.tv_usec = 0;
    setsockopt(server_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    long response_length = 0;
    int retn = 0;
    int host_sock = -8;
    int persistent_connect = 0;
    int checkflag = 0;
    int has_contentlength_flag = 0;
    //--------------------------------------
    struct sockaddr_in hostaddr;
    struct hostent *hp;
    char cmd1[TEMP_SIZE];
    char cmd2[TEMP_SIZE];
    char cmd3[TEMP_SIZE];
    char full_buf[REQUERST_SIZE];
    char browser_buf[BUFFER_SIZE];

    char log_responsecode[8];
    char log_firstline[128];
    char remote_server_host[128];
    char proxy_ip[32];
    char client_ip[32];
    char forwardstr[128];
    //--------------------------------------

    // get request from client
    bzero(browser_buf, BUFFER_SIZE);
    bzero(full_buf, REQUERST_SIZE);
    while(read(server_sock, browser_buf, BUFFER_SIZE) > 0){
        strcat(full_buf, browser_buf);
        bzero(browser_buf, BUFFER_SIZE);
        if(strstr(full_buf, "\r\n\r\n") == NULL) {
            continue;
        }else {
            bzero(cmd1, TEMP_SIZE);
            bzero(cmd2, TEMP_SIZE);
            bzero(cmd3, TEMP_SIZE);
            bzero(log_firstline, 128);
            bzero(remote_server_host, 128);
            persistent_connect = 0;
            // process request 
            sscanf(full_buf, "%s %s %s", cmd1, cmd2, cmd3);
            if(strcmp(cmd1, "GET") != 0 && strcmp(cmd1, "HEAD") != 0) {
                proxyResponseError(server_sock, 501, cmd3);
                bzero(full_buf, REQUERST_SIZE); 
                continue;
            }
            if(strcmp(cmd3, "HTTP/1.1") != 0 && strcmp(cmd3, "HTTP/1.0") != 0) {
                proxyResponseError(server_sock, 400, cmd3);
                bzero(full_buf, REQUERST_SIZE);
                continue;
            }
            if(strcmp(cmd3, "HTTP/1.1") == 0) {
                persistent_connect = 1;
            }
            retn = 0;
            while(strlen(full_buf) > retn){
                if(full_buf[retn] == '\r') {
                    strncpy(log_firstline, full_buf, retn);
                    break;
                }
                retn++;
            }
            checkflag = 0;
            while(strlen(full_buf) > retn){
                if(full_buf[retn] == '\r') {
                    bzero(cmd1, TEMP_SIZE);
                    bzero(cmd2, TEMP_SIZE);
                    sscanf(full_buf+retn, "%s%s", cmd1, cmd2);
                    if(strcmp(cmd1, "Host:") == 0){
                        strcpy(remote_server_host, cmd2);
                        checkflag = 1;
                    }
                    if(persistent_connect == 1 || (strcmp(cmd1, "Proxy-Connection:") == 0 && strcmp(cmd2, "Keep-Alive") == 0)){
                        persistent_connect = 1;
                    }
                }
                retn++;
            }
            
            if(checkflag == 0) {
                proxyResponseError(server_sock, 400, cmd3);
                bzero(full_buf, REQUERST_SIZE); 
                continue;
            }else{
                //filter
                checkflag = 0;
                for(int i = 0; i < sites->occupied; i++) {
                    if(strcmp(remote_server_host, getSiteItem(sites, i)) == 0) {
                        checkflag = 1;
                        break;
                    }
                }
                if (checkflag == 1) {
                    proxyResponseError(server_sock, 403, cmd3);
                    bzero(full_buf, REQUERST_SIZE);
                    break;
                }
            }
            //--------------------------------------------
            if(host_sock < 0) {
                //get local ip
                bzero(proxy_ip, 32);
                getlocalIP(proxy_ip);
                //get client ip
                bzero(client_ip, 32);
                getclientIP(client_ip, server_sock);
                //add forward message 
                bzero(forwardstr, 128);
                sprintf(forwardstr, "Forwarded: for=%s; proto=http; by=%s\r\n\r\n", client_ip, proxy_ip);
                hp = gethostbyname(remote_server_host);
                if(hp == NULL) {
                    proxyResponseError(server_sock, 400, cmd3);
                    bzero(full_buf, REQUERST_SIZE); 
                    continue;
                }

                bzero((char *)&hostaddr, sizeof(hostaddr));
                hostaddr.sin_family = AF_INET; 
                bcopy((char *)hp->h_addr_list[0],(char *)&hostaddr.sin_addr.s_addr, hp->h_length);
                hostaddr.sin_port = htons(80);

                if((host_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
                    failHandler("create socket error!");
                }
                struct timeval tv2;
                tv2.tv_sec = 60;
                tv2.tv_usec = 0;
                setsockopt(host_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv2, sizeof tv2);
                //connect remote server
                if(connect(host_sock, (struct sockaddr *)&hostaddr, (socklen_t)sizeof(hostaddr)) < 0) {
                    failHandler("connect host socket error!");
                }
            }
            //add forward message
            strcpy(&full_buf[strlen(full_buf)-2], forwardstr);
            //forward request 
            if(write(host_sock, full_buf, REQUERST_SIZE) < 0){
                close(server_sock);
                close(host_sock);
                return;
            } 
            //--------------------------------------------
            //get resoponse and forward to client
            checkflag = 1;
            response_length = 0;
            has_contentlength_flag = 0;
            bzero(browser_buf, BUFFER_SIZE);
            while(read(host_sock, browser_buf, BUFFER_SIZE) > 0) {
                if (checkflag) {
                    checkflag = 0;
                    bzero(cmd1, TEMP_SIZE);
                    bzero(log_responsecode, 8);
                    bzero(cmd3, TEMP_SIZE);
                    sscanf(browser_buf, "%s %s %s", cmd1, log_responsecode, cmd3);
                    retn = 0;
                    bzero(cmd1, TEMP_SIZE);
                    bzero(cmd2, TEMP_SIZE);
                    while(strlen(browser_buf) > retn){
                        if(browser_buf[retn] == '\r') {
                            sscanf(browser_buf+retn, "%s%s", cmd1, cmd2);
                            if(strcmp(cmd1, "Content-Length:") == 0){
                                response_length = atol(cmd2);
                                has_contentlength_flag = 1;
                                break;
                            }
                        }
                        retn++;
                    }
                    if (has_contentlength_flag) {
                        remotelog(client_ip,log_firstline,log_responsecode,response_length);
                    }else {
                        response_length = strlen(strstr(browser_buf, "\r\n\r\n")) - 4;
                    }  
                }else{
                    if(has_contentlength_flag == 0) {
                        response_length += strlen(browser_buf);
                    }    
                }
                if(write(server_sock, browser_buf, BUFFER_SIZE) < 0) {
                    close(server_sock);
                    close(host_sock);
                    return;
                }
                bzero(browser_buf, BUFFER_SIZE);
            }
            if(has_contentlength_flag == 0){
                remotelog(client_ip,log_firstline,log_responsecode,response_length);
            }
        }
        bzero(full_buf, REQUERST_SIZE); 
        if(persistent_connect == 0) {
            break;
        }
    }
    close(server_sock);
    if(host_sock > 0) {
        close(host_sock);
    }    
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

    if(listen(proxy_sock, 20) < 0){
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
