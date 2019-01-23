#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "util.h"
#include "mysocket.h"

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
    int server_sock = build_server_socket(atoi(argv[1]), "Sever Fail: Cannot establish socket connection. End of client.");
    
    

    close(server_sock);
    return 0;
}
