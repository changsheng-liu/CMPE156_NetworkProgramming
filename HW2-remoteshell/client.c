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
    int client_sock = build_client_socket(argv[1], atoi(argv[2]), "Client Fail: Cannot establish socket connection. End of client.");

    int ret;
    char buf[1024];
    char * input;
    while(1){
        input = readline("client $ ");
        add_history(input);
        
        if (write(client_sock, input , sizeof(input)) < 0) {
            close(client_sock);
            failHandler("Fail: cannot send request");
        }
        if(strcmp(input, "exit") == 0){
            close(client_sock);
            break;
        }
        while((ret = read(client_sock, buf, sizeof(buf))) > 0){
            buf[ret] = 0x00;
            if(strcmp(buf,"1234567") == 0) {
                break;
            }else{
                printf("%s",buf);
            }
        }
    }
    
    return 0;
}
