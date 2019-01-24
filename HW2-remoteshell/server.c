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
    printf("0000");
    if(argc != 2) {
        failHandler("Please use server by correct input!");
    }

    if(!isValidPort(argv[1])) {
        failHandler("Please use legal port number!");
    }
printf("1111");
    //build server socket and listen socket
    int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    printf("2222");
    int server_sock = build_server_socket(&listen_sock , atoi(argv[1]), "Sever Fail: Cannot establish socket connection. End of client.");
    
    int length = 1;
    printf("3333");
    
    char buf[1024];
    int ret;
    // while(1) {
                    printf("4444");

        while((ret = read(server_sock, buf, sizeof(buf)-1)) > 0){
            printf("hello");
            buf[ret] = 0x00;
            printf("%s",buf);
        }
        if(strlen(buf)>0){
            execute(buf, &server_sock ,&length);
            printf("\n%d\n", length);
        }
        printf("5555");
    // }
    

    close(listen_sock);
    close(server_sock);
    
    return 0;
}
