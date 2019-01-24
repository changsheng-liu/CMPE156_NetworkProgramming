#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include "util.h"
#include "mysocket.h"
#include "executer.h"

int main(int argc, char const *argv[])
{
    // check param is legal
    if(argc != 2) {
        failHandler("Please use server by correct input!");
    }

    if(!isValidPort(argv[1])) {
        failHandler("Please use legal port number!");
    }
    //build server socket and listen socket
    int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    int server_sock = build_server_socket(&listen_sock , atoi(argv[1]), "Sever Fail: Cannot establish socket connection. End of client.");
    
    int length = 1;
    
    char buf[1024];
    int ret;
    while((ret = read(server_sock, buf, sizeof(buf)-1)) > 0){
        if(strcmp(buf, "exit") == 0){
            close(server_sock);
            break;
        }
        buf[ret] = 0x00;
        execute(buf, &server_sock ,&length);
        printf("\n%d\n",length);
        write(server_sock, "1234567", 7);
    }
     
        // while((ret = recv(server_sock, buf, sizeof(buf), 0)) > 0){
        //     // buf[ret] = 0x00;
        //     printf("%s",buf);
        // }
        // 
        // if(strlen(buf)>0){
    
            // 
        // }
    // }

    // close(listen_sock);
    
    return 0;
}
