#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "util.h"
#include "myprotocol.h"
#include "client_util.h"

void check_param(int argc, char* argv[]) {
	if(argc != 4) {
        failHandler("Please use client by correct input! usage: ./client <server-ip> <server-port> <client-ip>");
    }
    if(!isValidIP(argv[1])) {
        failHandler("Please input correct server ip address!");
    }
    if(!isNumber(argv[2])) {
        failHandler("Please input correct port number!");
    }
    if(check_user_name_length(argv[3]) == 0){
        failHandler("Please input the user name less than 15 characters!");
    }
    if(isOnlyLettersOrNumbers(argv[3]) == 0) {
        failHandler("Please input the user name with only letters or numbers!");
    }
}

int check_name_conflict(char * my_name, int socket) {
    char buf[BUFFER_SIZE];
    bzero(buf, BUFFER_SIZE);
    sprintf(buf, "a:%s:", my_name);
    write(socket, buf, BUFFER_SIZE);
    bzero(buf, BUFFER_SIZE);
    if(read(socket, buf, BUFFER_SIZE) > 0) {
        if(strcmp(buf, CMD_CONFIRM_JOIN) == 0){
            return 1;
        }else if (strcmp(buf, CMD_REJECTION_JOIN) == 0) {
            return 0;
        }else{
            return -1;
        }
    }
    return -1;
}


void print_waiting_list(char * list) {
    int idx = 1;
	int i = 2;
	int j = 3;
	char name[CLIENT_NAME_LENGTH];
	strncpy(name,list+2, 2);
	while(j < strlen(list)-1){
		if(list[j] == ':') {
            bzero(name, CLIENT_NAME_LENGTH);
			strncpy(name,list+i, j-i);
			i = j+1;
			printf("%d) %s\n",idx, name);
			idx++;
		}
		j++;
	}
}

void process_waiting_list(int server_fd) {
    char buf[BUFFER_SIZE];
    bzero(buf, BUFFER_SIZE);
    char *whole_list = malloc(BUFFER_SIZE);
    bzero(whole_list, BUFFER_SIZE);
    int rt = 0;
    int idx = 0;
    int total = 0;
    while((rt = read(server_fd, buf, BUFFER_SIZE)) > 0) {
        buf[rt] = 0x00;
        idx++;
        if(strcmp(buf, "l::") == 0 && idx == 1) {
            printf("%s",no_available_client_msg);
            return;
        }else if (idx == 1){
            strcpy(whole_list, buf);
            total = rt;
        }else{
            whole_list = realloc(whole_list, idx*BUFFER_SIZE);
            strcat(whole_list, buf);
            total = total + rt;
        }
        whole_list[total] = 0x00;
        if(strstr(buf, "::") > 0) {
            break;
        }
        bzero(buf, BUFFER_SIZE);
    }
    print_waiting_list(whole_list);
    bzero(whole_list, strlen(whole_list));
    idx = 0;
    total = 0;
}

void print_received_msg(int socket) {
    char buf[BUFFER_SIZE];
    while(read(socket, buf, BUFFER_SIZE) > 0){
        printf("%s\n", buf);
    }
}

int should_command_work_with_state(char * cmd, client_state_t state) {
    if(state == CLIENT_STATE_NORMAL) {
        return 1;
    }else if(state == CLIENT_STATE_TALKING) {
        if(strcmp(cmd, "/connect") == 0 || strcmp(cmd, "/wait") == 0 || strcmp(cmd, "/list") == 0) {
            return 0;
        }
        return 1;
    }else if(state == CLIENT_STATE_WAITING) {
        if(strcmp(cmd, "/connect") == 0 || strcmp(cmd, "/wait") == 0 || strcmp(cmd, "/list") == 0) {
            return 0;
        }
        return 1;
    }
    return 0;
}

void send_msg_to_peer(int peer_fd, char * msg, char * my_name) {
    char * sending_msg = malloc(BUFFER_SIZE);
    bzero(sending_msg, BUFFER_SIZE);
    sprintf("%s: %s\n", my_name, msg);
    write(peer_fd, sending_msg, strlen(sending_msg));
}