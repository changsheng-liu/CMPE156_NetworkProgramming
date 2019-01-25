#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
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
    const char * server_build_fail_message = "Sever Fail: Cannot establish socket connection. End of server.";
    
    int lis_sock = init_socket(server_build_fail_message);;

    struct sockaddr_in addr = init_address(INADDR_ANY, atoi(argv[1]));

    server_socket_bind_listen(lis_sock, (struct sockaddr *) &addr, 1, server_build_fail_message);
    
    int server_sock;
    int length = 1;

    char buf[1024];
    int ret;
    
    while(1) {
        server_sock = server_socket_accept(lis_sock, (struct sockaddr *) &addr, server_build_fail_message);
        
        while((ret = read(server_sock, buf, sizeof(buf)-1)) > 0){
            if(strcmp(buf, "exit") == 0){
                break;
            }
            buf[ret] = 0x00;
            execute(buf, &server_sock ,&length);
            server_send_response_end(server_sock);
        }

        close(server_sock);
    }
    
    
    
    return 0;
}
