#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "util.h"
#include "mysocket.h"

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

    int ret;
    char buf[BUFFER_SIZE];
    char * input;
    while(1){
        input = readline("client $ ");
        add_history(input);
        if (write(client_sock, input , strlen(input)) < 0) {
            close(client_sock);
            failHandler("Fail: cannot send request");
        }
        if(strcmp(input, "exit") == 0){
            close(client_sock);
            break;
        }
        while((ret = read(client_sock, buf, sizeof(buf)-1)) > 0){
            buf[ret] = 0x00;
            if(client_receive_response_end(client_sock, buf)) {
                break;
            }else{
                printf("%s",buf);
            }
        }
    }
    
    return 0;
}
