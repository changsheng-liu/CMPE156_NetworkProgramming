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
#include "mysocket.h"

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
	// test for info, need remove 
	// char str[30];
	// for(int i = 0; i < server_info_list->occupied; i++) {
	// 	server_info_t * item = getServerItem(server_info_list, i);
	// 	struct sockaddr_in * addr = item->addr;
	// 	printf("port : %d \n", (int)ntohs(addr->sin_port));
	// 	printf("ip : %s \n",inet_ntop(AF_INET,&addr->sin_addr ,str, INET_ADDRSTRLEN));
	// }
	//***********************

	//udp basic
    int client_fd;

    if((client_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        failHandler("create socket error!");
    }

    struct sockaddr_in * addr = getServerItem(server_info_list, 0);
    socklen_t len = sizeof(*addr);
    char write_buf[BUFFER_SIZE];
    bzero(write_buf, BUFFER_SIZE);
    sprintf(write_buf, "%s %s", CMD_CHECKFILE, target_file);
    sendto(client_fd, write_buf, BUFFER_SIZE, 0, (struct sockaddr*)addr, len);

    server_response_t * read_buf = malloc(sizeof(server_response_t));
    memset(read_buf, 0, sizeof(server_response_t));
    recvfrom(client_fd, read_buf, sizeof(server_response_t), 0, NULL, NULL);
    if(strcmp(read_buf->cmd, CMD_CHECKFILE) == 0) {
        if(read_buf->have_file_flag) {
            printf("Find file! Downloading...\n");
            bzero(write_buf, BUFFER_SIZE);
            sprintf(write_buf, "%s %s %d %ld", CMD_DOWNLOAD, target_file, 0, read_buf->file_length);
            sendto(client_fd, write_buf, BUFFER_SIZE, 0, (struct sockaddr*)addr, len);

            memset(read_buf, 0, sizeof(server_response_t));
            
            
            FILE * fp;
        	char name[100]; 
	        sprintf(name,"output.txt");
	        // remove(name);
	        if((fp = fopen(name, "w+")) == NULL) {
		        failHandler("Fail open file!");
	        }
            int ret;
	        while((ret = read(client_fd, read_buf, sizeof(server_response_t))) > 0){ 
		        if (strcmp(read_buf->cmd, CMD_DOWNLOAD) == 0) {
			        fprintf(fp, "%s", read_buf->file_content);
			        fflush(fp); 	
			        if(read_buf->content_is_end == 1) break;
		        }
	        }
	        fclose(fp);
        
        
        }else{
            printf("No such file! Exiting...\n");
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
		if(strcmp(pre_port, port) == 0 && strcmp(pre_ip, ip) == 0) {continue;}
		strcpy(pre_ip, ip);
		strcpy(pre_port, port);

		struct sockaddr_in *ser_addr = malloc(sizeof(struct sockaddr_in));
		// bzero(ser_addr, sizeof(ser_addr));
		memset(ser_addr, 0, sizeof(struct sockaddr_in));
		ser_addr->sin_family = AF_INET;
		ser_addr->sin_port = htons(atoi(port));  
		if(inet_aton(ip, &ser_addr->sin_addr)<=0) { 
			failHandler("bind socket error!");
		} 
		addServerItem(inner_server_info_list, ser_addr);
	}
	fclose(fp);
	return inner_server_info_list;
}