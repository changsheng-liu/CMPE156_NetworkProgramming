#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include "util.h"
#include "myprotocol.h"

char * my_name;
int is_talking_flag;
int server_fd;
int talk_fd;

int talk_port;
pthread_t talk_thread;

const char * invalid_input_msg = "Invalid input! Please input command or talk to someone!\n";

void getlocalIP(char * str) {
    char proxyHostName[255];
    gethostname(proxyHostName, 255);
    struct hostent * proxyHost;
    proxyHost=gethostbyname(proxyHostName);
    inet_ntop(AF_INET,proxyHost->h_addr_list[0] ,str, INET_ADDRSTRLEN);
}

void create_wait_socket() {
    int listen_fd;
    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        failHandler("create socket error!");
    }

    struct sockaddr_in * addr = malloc(sizeof(struct sockaddr_in));
    memset(addr, 0, sizeof(struct sockaddr_in));
    // bzero(addr, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = htonl(INADDR_ANY);
    addr->sin_port = htons(0);

    if((bind(listen_fd, (struct sockaddr*)addr, (socklen_t)sizeof(struct sockaddr_in))) < 0) {
        failHandler("bind socket error!");
    }
    
    socklen_t len = sizeof(*addr);
    if (getsockname(listen_fd, (struct sockaddr *)addr, &len) == -1) {
		failHandler("getsockname() failed");
	}
    talk_port = ntohs(addr->sin_port);
}

void print_waiting_list(char * list) {
    int idx = 1;
	int i = 2;
	int j = 3;
	char name[CLIENT_NAME_LENGTH];
	strncpy(name,list+2, 2);
	while(j < strlen(list)-1){
		if(list[j] == ':') {
			strncpy(name,list+i, j-i);
			i = j+1;
			printf("%d) %s\n",idx, name);
			idx++;
		}
		j++;
	}
}

void build_talk_connection(char * peer_name, char * ip, int port) {
    if((talk_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        failHandler("create talk socket error!");
    }
	struct sockaddr_in * addr = malloc(sizeof(struct sockaddr_in));
    memset(addr, 0, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    if(inet_aton(ip, &addr->sin_addr)<=0) { 
        failHandler("init talk socket addr error!");
    } 
    addr->sin_port = htons(port);

	if(connect(server_fd, (struct sockaddr*)addr, (socklen_t)sizeof(struct sockaddr_in)) < 0){
        failHandler("connect talk socket error!");
    }
    is_talking_flag = 1;
}

void receive_msg(int socket) {
    char buf[BUFFER_SIZE];
    while(read(socket, buf, BUFFER_SIZE) > 0){
        printf("%s\n", buf);
        //todo: print peer name
    }
}

void * peer_talk(void * arg) {
    if (pthread_detach(pthread_self()) != 0)
        exit(1);
    int * sock = (int *)arg;
    receive_msg(*sock);
    pthread_exit(NULL);
}

void create_receive_thread() {
    if (pthread_create(&talk_thread, NULL, peer_talk, (void *)&talk_fd) != 0) {
        failHandler("create thread error!");
    }
}

void read_waiting_list(int server_fd) {
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
}

void read_connect_peer(int server_fd) {
    char buf[BUFFER_SIZE];
    bzero(buf,BUFFER_SIZE);
    if(read(server_fd, buf, BUFFER_SIZE) > 0) {
        if(strcmp(buf, "c::") == 0) {
            printf("%s",no_such_client_msg);
        }else{
            strtok(buf, ":");
            char * peer_name = strtok(NULL, ":");
            char * peer_ip = strtok(NULL, ":");
            char * peer_port = strtok(NULL, ":");
            build_talk_connection(peer_name, peer_ip, atoi(peer_port));
            create_receive_thread();
        }
    }
}

void process_input(int server_fd) {
    char * input;
    char * cmd;

    char * buf;
    int buf_length;
   
    while(1){
        input = readline("> ");
        if('/' == input[0]) {
            cmd = strtok(input, " ");
            if(strcmp(cmd, "/wait") == 0) {
                create_wait_socket();//TODO 
                buf = format_wait_cmd(my_name, talk_port, &buf_length);
            }else if(strcmp(cmd, "/list")  == 0) {
                buf = format_list_cmd(my_name, &buf_length);
            }else if(strcmp(cmd, "/connect") == 0) {
                char * peername = strtok(NULL, "");
                if(strcmp(peername, my_name) == 0) {
                    printf("%s",talk_self_msg);
                    continue;
                }
                buf = format_connect_cmd(cmd, peername, &buf_length);
            }else if(strcmp(cmd, "/quit") == 0) {
                buf = format_quit_cmd(my_name, &buf_length);
            }else {
                printf("%s",wrong_cmd_msg);
                continue;
            }

            write(server_fd, buf, buf_length);

            if(strcmp(cmd, "/wait") == 0) {
                //TODO need to block?
                continue;
            }else if(strcmp(cmd, "/list") == 0) {
                read_waiting_list(server_fd);
            }else if(strcmp(cmd, "/connect") == 0) {
                read_connect_peer(server_fd);
            }else if(strcmp(cmd, "/quit") == 0) {
                close(server_fd);
                return;
            }
        }else{
            if(is_talking_flag) {
                write(talk_fd, input, strlen(input));
            }else{
                printf("%s",invalid_input_msg);
                continue;
            }
        }
    }
}

void checkParam(int argc, char* argv[]) {
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

int main(int argc, char* argv[]) {
	checkParam(argc, argv);
    my_name = argv[3];
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        failHandler("create socket error!");
    }
	struct timeval tv;
    tv.tv_sec = 180;
    tv.tv_usec = 0;
    setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
	
	struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    if(inet_aton(argv[1], &addr.sin_addr)<=0) { 
        failHandler("connect socket error!");
    } 
    addr.sin_port = htons(atoi(argv[2]));

	if(connect(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        failHandler("connect socket error!");
    }
	
    int ret = check_name_conflict(my_name, server_fd);
    if(ret == 0) {
        printf("User name conflict! Exiting...\n");
        return 0;
    }
    //business logic
    process_input(server_fd);
    
    return 0;
}

