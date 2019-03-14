#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "myprotocol.h"
#include "util.h"
#include "dynamicarray.h"

#define MAX_LISTENING_QUEUE 20
#define SERVER_BUFF_SIZE 1024

client_list_t * clients;
pthread_mutex_t clients_lock = PTHREAD_MUTEX_INITIALIZER;
client_name_list_t * client_names;
pthread_mutex_t client_names_lock = PTHREAD_MUTEX_INITIALIZER;

void sendclient(int server_fd, client_t *c, char * buf) {
    bzero(buf, SERVER_BUFF_SIZE);
    sprintf(buf, "c:%s:%s:%s:%d:", CMD_CONNECT, c->client, c->ip, c->port);
    write(server_fd, buf, SERVER_BUFF_SIZE);
}

void command_response(int server_fd) {
    char buf[SERVER_BUFF_SIZE];
    //TODO
    //2. need to add end of peer
    //3. empty list
    bzero(buf, SERVER_BUFF_SIZE);
    while(read(server_fd, buf, sizeof(char) * SERVER_BUFF_SIZE) > 0) {
        printf("%s\n", buf);
        char * command = strtok(buf, ":");
        if(strcmp(command, CMD_JOIN) == 0) {
            char * clientname = strtok(NULL, ":");
            if(hasNameItem(client_names,clientname) == 1) {
                write(server_fd, CMD_CONFIRM_JOIN, 6);
                close(server_fd);
                break;
            }else{
                pthread_mutex_lock(&client_names_lock);
                addNameItem(client_names, clientname);
                pthread_mutex_unlock(&client_names_lock);
                write(server_fd, CMD_CONFIRM_JOIN, 6);
            }
        }else if(strcmp(command, CMD_QUIT) == 0) {
            char * clientname = strtok(NULL, ":");
            pthread_mutex_lock(&clients_lock);
            int idx = findItem(clients, clientname);
            if (idx >= 0) {
                removeItem(clients, idx);
            }
            pthread_mutex_unlock(&clients_lock);
            pthread_mutex_lock(&client_names_lock);
            removeNameItem(client_names, clientname);
            pthread_mutex_unlock(&client_names_lock);
            close(server_fd);
            break;
        }else if(strcmp(command, CMD_CONNECT) == 0) {
            char * clientname = strtok(NULL, ":");
            char * peername = strtok(NULL, ":");
            pthread_mutex_lock(&clients_lock);
            int idx = findItem(clients, peername);
            if (idx >= 0) {
                client_t * c = popItem(clients, idx);
                pthread_mutex_unlock(&clients_lock);
                sendclient(server_fd, c, buf);
            }else{
                pthread_mutex_unlock(&clients_lock);
                bzero(buf, SERVER_BUFF_SIZE);
                strcpy(buf, no_such_client_msg);
                write(server_fd, "c::", 4);
            }
        }else if(strcmp(command, CMD_WAIT) == 0) {
            client_t *c = malloc(sizeof(client_t));
            memset(c, 0, sizeof(client_t));

            char * clientname = strtok(NULL, ":");
            char * ip = strtok(NULL, ":");
            char * port_str = strtok(NULL, ":");
            int port = atoi(port_str);
            strcpy(c->client, clientname);
            strcpy(c->ip, ip);
            c->port = port;
            
            pthread_mutex_lock(&clients_lock);
            addItem(clients, c);
            pthread_mutex_unlock(&clients_lock);
        }else if(strcmp(command, CMD_LIST) == 0) {
            int list_length = 0;
            char *listbuf = printList(clients, &list_length);
            write(server_fd, listbuf, list_length);
        }else{
            bzero(buf, SERVER_BUFF_SIZE);
            strcpy(buf, wrong_cmd_msg);
            write(server_fd, buf, SERVER_BUFF_SIZE);
        }
        bzero(buf, SERVER_BUFF_SIZE);
    }
}

void * client_handler(void * arg) {
    if (pthread_detach(pthread_self()) != 0)
        exit(1);
    int * fd = (int *)arg;
    command_response(*fd);
    pthread_exit(NULL);
}

void checkParam(int argc, char* argv[]) {
    if(argc != 2) {
        failHandler("Please use server by correct input! usage: ./server <portnumber>");
    }
    if(!isValidPort(argv[1])) {
        failHandler("Please use legal port number!");
    }
}

int main(int argc, char* argv[]) {
    checkParam(argc, argv);
    clients = initArray();
    client_names = initNameArray();
    int listen_fd;
    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        failHandler("create socket error!");
    }

    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    if((bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr))) < 0) {
        failHandler("bind socket error!");
    }
    
    if(listen(listen_fd, MAX_LISTENING_QUEUE) < 0){
        failHandler("listen socket error!");
    }
    int addr_len;
    while (1) {
		addr_len = sizeof(addr);
		int server_fd = accept(listen_fd, (struct sockaddr *)&addr, (socklen_t *) &addr_len);
		if(server_fd < 0) {
			failHandler("accopt socket error!");		
		}
		//business logic
        pthread_t thread;
        // command_response(server_fd);
        if (pthread_create(&thread, NULL, client_handler, (void *)&server_fd) != 0) {
            failHandler("create thread error!");
        }
	}

    return 0;
}
