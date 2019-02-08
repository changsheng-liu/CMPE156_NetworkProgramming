#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "util.h"
#include "mysocket.h"
#include <signal.h>
#include <pthread.h>

#define MAX_THREAD_SIZE 10
#define MAX_LINE_SIZE 2048

typedef struct s_info_t{
	int port;
	char IP[16];
	struct s_info_t * next;
}server_info_t;

server_info_t * info_head_dummy;
int total_lines = 0;

void read_server_info(const char * info) {
    info_head_dummy = malloc(sizeof(server_info_t));
	server_info_t * cur = info_head_dummy;
	FILE *fp;
	fp = fopen(info,"r");
	if(fp == NULL){
		failHandler("Read file fail!");
	}
	char strLine[MAX_LINE_SIZE];
	char * ip;
	char * port;
	while (!feof(fp) && !ferror(fp)) {
		strcpy(strLine, "\n"); 
		fgets(strLine, sizeof(strLine), fp);
		if (strlen(strLine) > 7) {
			cur->next = malloc(sizeof(server_info_t));
			cur = cur->next;
			ip = strtok(strLine, " ");
			strcpy(cur->IP, ip);
			port = strtok(NULL, "");
			cur->port = atoi(port);
			cur->next = NULL;
			total_lines++;
		}
	}
	fclose(fp);
}

void clean_server_info(void) {
	server_info_t * cur = info_head_dummy;
	while (cur != NULL) {
		info_head_dummy = cur->next;
		free(cur);
		cur = info_head_dummy;
	}
}

void decide_chrunk(void){
	long file_size = 4312305664;

}

void unpackage_file(int sock){
	int ret; 
	struct server_package * read_buf = malloc(sizeof(struct server_package));
	memset(read_buf, 0, sizeof(struct server_package));

	FILE * fp;
	if((fp = fopen("output1.dat", "w+")) == NULL) {
		fprintf(stderr,"Fail: cannot open target file!\n");
	}
	while((ret = read(sock, read_buf, sizeof(struct server_package))) > 0){ 
		if (strcmp(read_buf->cmd, CMD_DOWNLOAD) == 0) {
			fprintf(fp, "%s", read_buf->file_content);
			fflush(fp); 	
			if(read_buf->content_is_end == 1) break;
		}
	}
	fclose(fp);
}

void unpackage_file2(int sock){
	int ret; 
	struct server_package * read_buf = malloc(sizeof(struct server_package));
	memset(read_buf, 0, sizeof(struct server_package));

	FILE * fp;
	if((fp = fopen("output2.dat", "w+")) == NULL) {
		fprintf(stderr,"Fail: cannot open target file!\n");
	}
	while((ret = read(sock, read_buf, sizeof(struct server_package))) > 0){ 
		if (strcmp(read_buf->cmd, CMD_DOWNLOAD) == 0) {
			fprintf(fp, "%s", read_buf->file_content);
			fflush(fp); 	
			if(read_buf->content_is_end == 1) break;
		}
	}
	fclose(fp);
}

void combine_file(void) {
		FILE * f1;
	if((f1 = fopen("output1.dat","a")) == NULL){
		fprintf(stderr, "error!");
		exit(0);
	}
	
	FILE * f2;
	if((f2 = fopen("output2.dat","r")) == NULL){
		fprintf(stderr, "error!");
		exit(0);
	}
	int ret;
	char buf[1];
	memset(buf, 0x00, sizeof(char) * 1);

	while((ret = fread(buf, 1, sizeof(char) * 1,f2)) >0 ){
		buf[ret] = 0x00;
		fwrite(buf, 1, sizeof(char) * 1,f1);
		memset(buf, 0x00, sizeof(char) * 1);
	}
	
	fclose(f1);
	fclose(f2);

}

int main(int argc, char const *argv[])
{
    // check param is legal
    if(argc != 4) {
        failHandler("Please use client by correct input! usage: ./client <server-info.txt> <num-connections> <filename>");
    }

    if(!hasFile(argv[1])) {
        failHandler("Please input correct server-info.txt!");
    }

    if(!isNumber(argv[2])) {
        failHandler("Please input correct num-connections!");
    }

    // create connection
    const char * server_info_file = argv[1];
    read_server_info(server_info_file);
    
    const char * target_file = argv[3];
    int max_num = atoi(argv[2]);
    const int connection_num = total_lines > max_num ? max_num : total_lines;

    // build client socket
    const char * client_build_fail_message = "Client Fail: Cannot establish socket connection. End of client.";

    int client_sock = init_socket(client_build_fail_message);

	server_info_t * effectIP = info_head_dummy->next;

    struct sockaddr_in addr = init_address(inet_addr(effectIP->IP), effectIP->port);
    client_socket_connect(client_sock, (struct sockaddr *)&addr, client_build_fail_message);
	
	// download process
	//1. check file 
	client_check_file(client_sock, target_file);
	int ret;
	struct server_package * read_buf = malloc(sizeof(struct server_package));
	memset(read_buf, 0, sizeof(struct server_package));

	if((ret = read(client_sock, read_buf, sizeof(struct server_package))) > 0){
    	const char * cmd = read_buf->cmd;
		if (strcmp(cmd, CMD_CHECKFILE) == 0) {  //check command if file is exist
			if (read_buf->have_file_flag == 1) {
				// download file
				decide_chrunk();
				
				client_download_file(client_sock, target_file, 0, 2000);

				unpackage_file(client_sock);

				client_download_file(client_sock, target_file, 2001, 4008);
				
				unpackage_file2(client_sock);

				combine_file();

			}else{
				socket_exit(client_sock);
				close(client_sock);
				return 0;
			}
		}
    }
	memset(read_buf,0,sizeof(struct server_package));
    close(client_sock);
    clean_server_info();
	free(read_buf);
    return 0;
}
