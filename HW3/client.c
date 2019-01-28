#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "util.h"
#include "mysocket.h"
#include <signal.h>

int s;
void crashHandler(int sig){ 
    write(s, "exit" , strlen("exit")); 
    close(s);
    exit(1);
}

int main(int argc, char const *argv[])
{
    // check param is legal
    if(argc != 3) {
        failHandler("Please use client by correct input!");
    }

    if(!isValidIP(argv[1])) {
        failHandler("Please use legal ip address!");
    }

    if(!isValidPort(argv[2])) {
        failHandler("Please use legal port number!");
    }

    // build client socket
    const char * client_build_fail_message = "Client Fail: Cannot establish socket connection. End of client.";

    int client_sock = init_socket(client_build_fail_message);

    struct sockaddr_in addr = init_address(inet_addr(argv[1]), atoi(argv[2]));

    client_socket_connect(client_sock, (struct sockaddr *)&addr, client_build_fail_message);
 
    s = client_sock;

    int ret;
    char * input;
    struct my_socket_package * read_buf = malloc(sizeof(struct my_socket_package));
    signal(SIGINT, crashHandler);
    while(1){
        
    }
    
    return 0;
}
