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

#define MAX_LISTENING_QUEUE 20

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
        close(server_fd);
	}

    return 0;
}
