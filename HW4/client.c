#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "util.h"
#include "mydatastructure.h"
#include "mysocket.h"

#define MAX_WORKER 100

job_list_t * done_jobs;
pthread_mutex_t done_job_lock = PTHREAD_MUTEX_INITIALIZER;

job_list_t * chunk_jobs;
pthread_mutex_t chunk_job_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t job_available = PTHREAD_COND_INITIALIZER;

server_list_t * server_info_list;
pthread_mutex_t server_info_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t server_available = PTHREAD_COND_INITIALIZER;

void checkParam(int argc, char* argv[]);
server_list_t * process_server_info(const char * info_list_file);
void * thread_download(void * param);
job_list_t * createJobsList(int job_num, const char * file_name, long total_size);
pthread_t * createWorkerList(int worker_size);
void combinefile(int chunk_num);

int main(int argc, char* argv[]) {
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

	//clarify variable
    int client_fd;
   	struct sockaddr_in * addr = getServerItem(server_info_list, 0);
    socklen_t len = sizeof(*addr);
    char write_buf[BUFFER_SIZE];
    server_response_t * read_buf = malloc(sizeof(server_response_t));
	pthread_t * threads;

	//memory clear / init 
    if((client_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        failHandler("create socket error!");
    }
	bzero(write_buf, BUFFER_SIZE);
	memset(read_buf, 0, sizeof(server_response_t));

	//start query file 
    sprintf(write_buf, "%s %s", CMD_CHECKFILE, target_file);
    sendto(client_fd, write_buf, BUFFER_SIZE, 0, (struct sockaddr*)addr, len);
    recvfrom(client_fd, read_buf, sizeof(server_response_t), 0, NULL, NULL);
	
	int has_file_flag = 0;
    if(strcmp(read_buf->cmd, CMD_CHECKFILE) == 0) {
        if(read_buf->have_file_flag) {
			// file exists, download...
            printf("Find file! Downloading...\n");
			has_file_flag = 1;
			chunk_jobs = createJobsList(chunk_num, target_file, read_buf->file_length);
			done_jobs = initJobArray(chunk_num);
			pthread_t * threads = createWorkerList(chunk_num > MAX_WORKER ?MAX_WORKER : chunk_num);
			int i;
			for(i = 0; i < chunk_num; i++) {
				pthread_join(threads[i], NULL);
			}
			combinefile(chunk_num);
        }else{
			// file not exists, end...
            printf("No such file! Exiting...\n");
        }
    }

    close(client_fd);
	deallocServerList(server_info_list);
	free(read_buf);
	if(has_file_flag > 0) {
		deallocJobList(done_jobs);
		deallocJobList(chunk_jobs);
		free(threads);
	}
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

job_list_t * createJobsList(int job_num, const char * file_name, long total_size) {
	job_list_t * jobs = initJobArray(job_num);
	int i;
	long single_length = total_size / job_num;

	for(i = 0; i < job_num; i++) {
		job_item_t * item = malloc(sizeof(job_item_t));
		item->job_id = i;
		item->file_name = file_name;
		item->file_start = i * single_length;
		if(i == job_num - 1){
			item->file_end = total_size - 1;
		}else{
			item->file_end = (i + 1) * single_length - 1;
		}
		addJobItem(jobs, item);
	}
	return jobs;
}

pthread_t * createWorkerList(int worker_size) {
	pthread_t *threads;
	threads = malloc(sizeof(pthread_t) * worker_size);
	int i;
	for(i = 0; i < worker_size; i++) {
		if(pthread_create(&threads[i], NULL, thread_download, NULL) != 0) {
			failHandler("create thread fail!");
		}
	}
	return threads;
}

void * thread_download(void * param) {
	// job_item_t * job = (job_item_t *)param;
	// if(job->file_end - job->file_start < 1) pthread_exit(NULL);

	//get port resource
	pthread_mutex_lock(&server_info_lock);
	while (server_info_list->occupied == 0) {
		pthread_cond_wait(&server_available, &server_info_lock);
	}
	struct sockaddr_in * addr = getServerItem(server_info_list, server_info_list->occupied-1);
	removeServerItem(server_info_list, server_info_list->occupied-1);
	pthread_mutex_unlock(&server_info_lock);

	int client_fd;
	if((client_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        failHandler("create socket error!");
    }
	char write_buf[BUFFER_SIZE];
    server_response_t * read_buf = malloc(sizeof(server_response_t));
	FILE * fp;
	char name[15]; 
	int ret;
    socklen_t len = sizeof(*addr);

	while(done_jobs->occupied != done_jobs->size) {
		//get undone task
		pthread_mutex_lock(&chunk_job_lock);
		if(chunk_jobs->occupied == 0) {
			pthread_mutex_unlock(&chunk_job_lock);
			close(client_fd);
			//release port resource
			pthread_mutex_lock(&server_info_lock);
			addServerItem(server_info_list, addr);
			pthread_mutex_unlock(&server_info_lock);
			pthread_cond_broadcast(&server_available);
			pthread_exit(NULL);
		}
		job_item_t * job = popJobItem(chunk_jobs);
		pthread_mutex_unlock(&chunk_job_lock);

		printf("file start %ld, end %ld\n", job->file_start, job->file_end);

		//send download command
		memset(read_buf, 0, sizeof(server_response_t));
		sprintf(write_buf, "%s %s %ld %ld", CMD_DOWNLOAD, job->file_name, job->file_start, job->file_end);
    	sendto(client_fd, write_buf, BUFFER_SIZE, 0, (struct sockaddr*)addr, len);

		//download and write into temp.x file
		sprintf(name, "temp.%d", job->job_id);
		if((fp = fopen(name, "w+")) == NULL) {
			failHandler("Fail open file!");
		}
		while((ret = recvfrom(client_fd, read_buf, sizeof(server_response_t), 0, NULL, NULL)) > 0){ 
			if (strcmp(read_buf->cmd, CMD_DOWNLOAD) == 0) {
				fprintf(fp, "%s", read_buf->file_content);
				fflush(fp); 	
				if(read_buf->content_is_end == 1) break;
			}
		}
		fclose(fp);

		// record as done job
		pthread_mutex_lock(&done_job_lock);
		addJobItem(done_jobs, job);
		pthread_mutex_unlock(&done_job_lock);
	}

	close(client_fd);

	//release port resource
	pthread_mutex_lock(&server_info_lock);
	addServerItem(server_info_list, addr);
	pthread_mutex_unlock(&server_info_lock);
	pthread_cond_broadcast(&server_available);
	
	pthread_exit(NULL);
}

void combinefile(int chunk_num) {
	int ret;
	char buf[1];
	memset(buf, 0x00, sizeof(char) * 1);
	FILE * targetfd;
	remove("./dest/result.txt");

	if((targetfd = fopen("./dest/result.txt","w+")) == NULL){
		failHandler("Fail open file!");
	}
	int i;
	char fn[15];
	FILE * fd;

	for(i = 0; i < chunk_num; i++){
		sprintf(fn, "temp.%d", i);
		if((fd = fopen(fn,"r")) == NULL){
			failHandler("Fail open file!");
		}
		while((ret = fread(buf, 1, sizeof(char) * 1,fd)) >0 ){
			buf[ret] = 0x00;
			fwrite(buf, 1, sizeof(char) * 1,targetfd);
			memset(buf, 0x00, sizeof(char) * 1);
		}
		fclose(fd);
		remove(fn);
	}
	fclose(targetfd);
}