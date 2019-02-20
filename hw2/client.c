#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
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
        input = readline("client $ ");

        if(strlen(input) > BUFFER_SIZE) {
            printf("Please input less that 1024 char length!\n");
            continue;
        }
        if (write(client_sock, input , strlen(input)) < 0) {
            close(client_sock);
            failHandler("Fail: cannot send request");
        }
        if(strcmp(input, "exit") == 0){
            close(client_sock);
            break;
        }

        memset(read_buf, 0, sizeof(struct my_socket_package));
        while((ret = read(client_sock, read_buf, sizeof(struct my_socket_package))) > 0){
            printf("%s",read_buf->message);
            if (read_buf->is_end) {
                break;
            }
            memset(read_buf,0,sizeof(struct my_socket_package));
        }
    }
    
    return 0;
}
