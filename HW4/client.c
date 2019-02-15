#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "util.h"
#include "mydatastructure.h"

#define BUFF_LEN 1024

server_list_t * server_info_list;

void checkParam(int argc, char* argv[]);
server_list_t * process_server_info(const char * info_list_file);

int main(int argc, char* argv[])
{
	checkParam(argc, argv);
	const char * target_file = argv[3];
    int chunk_num = atoi(argv[2]);
    server_info_list = process_server_info(argv[1]);
	
	//***********************
	//test for info, need remove 
	// for(int i = 0; i < server_info_list->occupied; i++) {
	// 	printf("port : %d \n", getServerItem(server_info_list, i)->port);
	// }
	//***********************

	//udp basic
    int client_fd;
    struct sockaddr_in ser_addr;

    if((client_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        failHandler("create socket error!");
    }
    bzero(&ser_addr, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    // ser_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    ser_addr.sin_port = htons(atoi(argv[1]));  
    if(inet_aton("127.0.0.1", &ser_addr.sin_addr)<=0) { 
        failHandler("bind socket error!");
    } 
    socklen_t len;
    struct sockaddr_in src;
    char buf[BUFF_LEN];
    char recv[BUFF_LEN];
    for( ; ; ) {
        while(fgets(buf, BUFF_LEN, stdin) != NULL){
            len = sizeof(ser_addr);
            sendto(client_fd, buf, BUFF_LEN, 0, (struct sockaddr*)&ser_addr, len);
            recvfrom(client_fd, recv, BUFF_LEN, 0, (struct sockaddr*)&src, &len);
            
            bzero(recv, BUFF_LEN);
        }
    }

    close(client_fd);

	deallocServerList(server_info_list);
    return 0;
}

void checkParam(int argc, char* argv[]) {
	if(argc != 4) {
        failHandler("Please use client by correct input! usage: ./client <server-info.txt> <num-connections> <filename>");
    }

    if(!hasFile(argv[1])) {
        failHandler("Please input correct server-info.txt!");
    }

    if(!isNumber(argv[2])) {
        failHandler("Please input correct num-connections!");
    }
}

server_list_t * process_server_info(const char * info_list_file) {
	FILE *fp;
	fp = fopen(info_list_file,"r");
	if(fp == NULL){
		failHandler("Read file fail!");
	}
	char ip[100];
	char port[100];
	char pre_ip[100];
	char pre_port[100];
	server_list_t * inner_server_info_list = initServerListArray();
	while (!feof(fp) && !ferror(fp)) {
		fscanf(fp, "%s%s", ip, port);
		server_info_t * info = malloc(sizeof(server_info_t));
		if(strcmp(pre_port, port) == 0 && strcmp(pre_ip, ip) == 0) {continue;}
		strcpy(info->IP, ip);
		info->port = atoi(port);
		addServerItem(inner_server_info_list, info);
		strcpy(pre_ip, ip);
		strcpy(pre_port, port);
	}
	fclose(fp);
	return inner_server_info_list;
}