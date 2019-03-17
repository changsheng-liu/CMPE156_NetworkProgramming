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
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include "util.h"
#include "myprotocol.h"
#include "client_util.h"

const char * invalid_input_msg = "Invalid input! Please input command or talk to someone!\n";
const char * state_not_match_msg = "Cound not handle this command in this state!\n";

char * my_name;
client_state_t my_state;

int talk_fd;
int l_fd;

//print peer relate functions
void * __peer_receive__(void * arg) {
    if (pthread_detach(pthread_self()) != 0)
        exit(1);
    char buf[BUFFER_SIZE];
    while(read(talk_fd, buf, BUFFER_SIZE) > 0){
        printf("%s\n", buf);
    }
    pthread_exit(NULL);
}

void create_receive_thread(int peer_talk_fd) {
    my_state = CLIENT_STATE_TALKING;
    pthread_t talk_thread;
    if (pthread_create(&talk_thread, NULL, __peer_receive__, NULL) != 0) {
        failHandler("create thread error!");
    }
}

//wait related functions
void * __listen_receive__(void * arg) {
    if (pthread_detach(pthread_self()) != 0)
        exit(1);
    
    fd_set rset;

    while(1){
        FD_ZERO(&rset);
        FD_SET(l_fd, &rset);
        if(my_state == CLIENT_STATE_NORMAL){
            break;
        }
        int ret = select(l_fd + 1, &rset, NULL, NULL,NULL); 
        if(ret < 0){
            failHandler("selsect error!");
        }else if(ret == 0){
            continue;
        }else if(ret > 0){
            talk_fd = accept(l_fd, (struct sockaddr*)NULL, NULL);
            create_receive_thread(talk_fd);
            close(l_fd);
            break;
        }
    }
   
    pthread_exit(NULL);
}

int wait_pre_process() {
    my_state = CLIENT_STATE_WAITING;
    int listen_fd;
    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        failHandler("create socket error!");
    }

    struct sockaddr_in * addr = malloc(sizeof(struct sockaddr_in));
    memset(addr, 0, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = htonl(INADDR_ANY);
    addr->sin_port = htons(0);

    if((bind(listen_fd, (struct sockaddr*)addr, (socklen_t)sizeof(struct sockaddr_in))) < 0) {
        failHandler("bind socket error!");
    }
    
    if(listen(listen_fd, 1) < 0){
        failHandler("listen socket error!");
    }

    pthread_t listen_thread;
    l_fd = listen_fd;
    if (pthread_create(&listen_thread, NULL, __listen_receive__, NULL) != 0) {
        failHandler("create thread error!");
    }

    socklen_t len = sizeof(*addr);
    if (getsockname(listen_fd, (struct sockaddr *)addr, &len) == -1) {
		failHandler("getsockname() failed");
	}
    return ntohs(addr->sin_port);
}

//connect related functions
void __connect_peer__(char * peer_name, char * ip, int port) {
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

	if(connect(talk_fd, (struct sockaddr*)addr, (socklen_t)sizeof(struct sockaddr_in)) < 0){
        failHandler("connect peer error!");
    }
    create_receive_thread(talk_fd);
}

void connect_after_process(int server_fd) {
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
            __connect_peer__(peer_name, peer_ip, atoi(peer_port));
        }
    }
}

//user input entrance
void process_input(int server_fd) {
    char * input;
    char * cmd;

    char * buf;
    int buf_length;
   
    while(1){
        input = readline("> ");
        if('/' == input[0]) {
            cmd = strtok(input, " ");
            if(should_command_work_with_state(cmd, my_state) == 0) {
                printf("%s",state_not_match_msg);
                continue;
            }
            if(strcmp(cmd, "/wait") == 0) {
                int talk_port = wait_pre_process(); 
                buf = format_wait_cmd(my_name, talk_port, &buf_length);
            }else if(strcmp(cmd, "/list")  == 0) {
                buf = format_list_cmd(my_name, &buf_length);
            }else if(strcmp(cmd, "/connect") == 0) {
                char * peername = strtok(NULL, "");
                if(strcmp(peername, my_name) == 0) {
                    printf("%s",talk_self_msg);
                    continue;
                }
                buf = format_connect_cmd(my_name, peername, &buf_length);
            }else if(strcmp(cmd, "/quit") == 0) {
                buf = format_quit_cmd(my_name, &buf_length);
            }else {
                printf("%s",wrong_cmd_msg);
                continue;
            }

            write(server_fd, buf, buf_length);

            if(strcmp(cmd, "/wait") == 0) {
                continue;
            }else if(strcmp(cmd, "/list") == 0) {
                process_waiting_list(server_fd);
            }else if(strcmp(cmd, "/connect") == 0) {
                connect_after_process(server_fd);//TODO
            }else if(strcmp(cmd, "/quit") == 0) {
                close(server_fd);
                if(my_state == CLIENT_STATE_TALKING) {
                    close(talk_fd);
                }
                return;
            }
        }else{
            if(my_state == CLIENT_STATE_TALKING) {
                send_msg_to_peer(talk_fd, input, my_name);
            }else{
                printf("%s",invalid_input_msg);
                continue;
            }
        }
    }
}

void INThandler(int sig) {
    if(my_state == CLIENT_STATE_TALKING) {
        shutdown(talk_fd, SHUT_RDWR);
    }
    my_state = CLIENT_STATE_NORMAL;
}

int main(int argc, char* argv[]) {
	check_param(argc, argv);
    my_name = argv[3];
    int server_fd;
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
        failHandler("init socket error!");
    } 
    addr.sin_port = htons(atoi(argv[2]));

	if(connect(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        failHandler("connect socket error!");
    }
	
    int ret = check_name_conflict(my_name, server_fd);
    if(ret == 0) {
        close(server_fd);
        failHandler("User name conflict! Exiting...");
    }
    
    my_state = CLIENT_STATE_NORMAL;
    signal(SIGINT, INThandler);
    
    //business logic entrance
    process_input(server_fd);
    
    return 0;
}