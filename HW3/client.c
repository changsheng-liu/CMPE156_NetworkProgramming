#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "util.h"
#include "mysocket.h"
#include <signal.h>
#include <pthread.h>

#define MAX_SOCK_SIZE 10

const char * client_build_fail_message = "Client Fail: Cannot establish socket connection. End of client.";

typedef struct s_info_t{
	int port;
	char IP[16];
	int idx;
	int valid_flag;
	struct s_info_t * next;
}server_info_t;

server_info_t * info_head_dummy;

struct thread_param {
	int thread_id;
	const char * filename;
	int file_part;
	int socketfd;
	long file_start;
	long file_end;
};

struct thread_param thread_param_array[MAX_SOCK_SIZE];


int read_server_info(const char * info) {
	int total_lines = 0;
    info_head_dummy = malloc(sizeof(server_info_t));
	server_info_t * cur = info_head_dummy;
	strcpy(cur->IP,"invalid");
	cur->port = -1;
	cur->valid_flag = -1;
	cur->idx = -1;
	FILE *fp;
	fp = fopen(info,"r");
	if(fp == NULL){
		failHandler("Read file fail!");
	}
	char ip[100];
	char port[100];
	while (!feof(fp) && !ferror(fp)) {
		fscanf(fp, "%s%s", ip, port);
		cur->next = malloc(sizeof(server_info_t));
		if(atoi(port) == cur->port && strcmp(cur->IP, ip) == 0) {continue;}
		cur = cur->next;
		strcpy(cur->IP, ip);
		cur->port = atoi(port);
		cur->idx = total_lines++;
		cur->valid_flag = 0;
	}
	fclose(fp);
	return total_lines;
}

void clean_server_info(void) {
	server_info_t * cur = info_head_dummy;
	while (cur != NULL) {
		info_head_dummy = cur->next;
		free(cur);
		cur = info_head_dummy;
	}
}

void download_single_part(int sock, int part){
	int ret; 
	struct server_package * read_buf = malloc(sizeof(struct server_package));
	memset(read_buf, 0, sizeof(struct server_package));

	FILE * fp;
	char * name; 
	sprintf(name,"output.%d",part);
	if((fp = fopen(name, "w+")) == NULL) {
		failHandler("Fail open file!");
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

void combine_file(int totalsize, const char * filename) {
	int ret;
	char buf[1];
	memset(buf, 0x00, sizeof(char) * 1);
	FILE * targetfd;
	if((targetfd = fopen("result.txt","w+")) == NULL){
		failHandler("Fail open file!");
	}
	int i;
	char fn[15];
	FILE * fd;

	for(i = 0; i < totalsize; i++){
		sprintf(fn, "output.%d", i);
		if((fd = fopen(fn,"r")) == NULL){
			failHandler("Fail open file!");
		}
		while((ret = fread(buf, 1, sizeof(char) * 1,fd)) >0 ){
			buf[ret] = 0x00;
			fwrite(buf, 1, sizeof(char) * 1,targetfd);
			memset(buf, 0x00, sizeof(char) * 1);
		}
		fclose(fd);
	}
	fclose(targetfd);

}

int build_connection(server_info_t * serverinfo) {
	int client_sock = init_socket(client_build_fail_message);
    struct sockaddr_in addr = init_address(inet_addr(serverinfo->IP), serverinfo->port);
    int connect_result = client_socket_connect(client_sock, (struct sockaddr *)&addr, client_build_fail_message);
	return connect_result < 0 ? -1 : client_sock;
}

void * thread_download(void * param){
	struct thread_param * myargv = (struct thread_param *)param;	
	if(myargv->socketfd <= 0) pthread_exit(param);
	struct client_package * write_buf = malloc(sizeof(struct client_package));;

	client_request_file(myargv->socketfd, myargv->filename, myargv->file_start, myargv->file_end,write_buf);
	download_single_part(myargv->socketfd, myargv->file_part);
	pthread_exit(param);
}

void download_file(int sockets[], const char * filename, long filesize, int threadsnum){
	pthread_t threads[MAX_SOCK_SIZE];
	int i;
	int valid_part = 0;
	long single_part_length = filesize / threadsnum;
	for(i = 0; i < MAX_SOCK_SIZE; i++){
		thread_param_array[i].thread_id = i;
		thread_param_array[i].socketfd = sockets[i];
		thread_param_array[i].filename = filename;
		if (sockets[i] < 0) {
			thread_param_array[i].file_end = -1;
			thread_param_array[i].file_start = -1;
			thread_param_array[i].file_part = -1;
		}else{
			thread_param_array[i].file_part = valid_part;
			thread_param_array[i].file_start = valid_part * single_part_length;
			if(valid_part == threadsnum - 1){
				thread_param_array[i].file_end = filesize - 1;
			}else{
				thread_param_array[i].file_end = (valid_part+1) * single_part_length - 1;
			}
			valid_part++;
		}
		
		if(pthread_create(&threads[i], NULL, thread_download, &thread_param_array[i]) != 0){
			failHandler("Thread fail!");
		}
	}
	void * status;
	for(i = 0; i < MAX_SOCK_SIZE; i++){
		pthread_join(threads[i], &status);
	}
	combine_file(threadsnum, filename);
}

int main(int argc, char const *argv[]){
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

    // preprocessing 
	const char * target_file = argv[3];
    int max_num = atoi(argv[2]);
    const char * server_info_file = argv[1];
    int valid_ips = read_server_info(server_info_file);
    
    int connection_num = valid_ips > max_num ? max_num : valid_ips;
	
    // build client sockets
	int sockets[MAX_SOCK_SIZE] = {-1};
	server_info_t * cur = info_head_dummy;
	int socketidx = 0;
	int client_sock;
	int valid_conn = 0;
	while(cur->next != NULL && socketidx < MAX_SOCK_SIZE && connection_num > 0){
		cur = cur->next;
		client_sock = build_connection(cur);
		sockets[socketidx] = client_sock;
		if(client_sock >= 0) {
			valid_conn++;
			connection_num--;
			cur->valid_flag = 1;
		}
		socketidx++;
	}
	
	
	if (valid_conn == 0) {
		failHandler("No valid connection!");
	}
	
	// download process
	//pick up a socket to check file
	socketidx = 0;
	while(socketidx < MAX_SOCK_SIZE){
		if(sockets[socketidx] <= 0){
			socketidx++;
			continue;
		}
		client_sock = sockets[socketidx];
		break;
	}
	struct server_package * read_buf = malloc(sizeof(struct server_package));
    struct client_package * write_buf = malloc(sizeof(struct client_package));;

	client_check_file(client_sock, target_file, write_buf);

	int ret;
	memset(read_buf, 0, sizeof(struct server_package));

	if((ret = read(client_sock, read_buf, sizeof(struct server_package))) > 0){
		if (strcmp(read_buf->cmd, CMD_CHECKFILE) == 0) { 
			//check command if file is exist 
			if (read_buf->have_file_flag == 1) {
				// server has the file, download file
				download_file(sockets, target_file, read_buf->file_length, valid_conn);
				printf("Downloading success! Closing sockets...\n");
			}else{
				//no such file
				printf("No such file! Closing sockets...\n");
			}
		}
    }
	
    socketidx = 0;
	while(socketidx < MAX_SOCK_SIZE){
		if (sockets[socketidx] >= 0) {
			socket_exit(sockets[socketidx], write_buf);
			close(sockets[socketidx]);	
		}
		socketidx++;
	}
	clean_server_info();
    return 0;
}
