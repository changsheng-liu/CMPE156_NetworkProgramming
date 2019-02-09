#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "util.h"
#include "mysocket.h"

int main(int argc, char const *argv[]) {
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
    int ret;
    struct client_package * read_buf = malloc(sizeof(struct client_package));
	struct server_package * write_buf = malloc(sizeof(struct server_package));

    while(1) {
        server_sock = server_socket_accept(lis_sock, (struct sockaddr *) &addr, server_build_fail_message);
        
        memset(read_buf, 0, sizeof(struct client_package));
        while((ret = read(server_sock, read_buf, sizeof(struct client_package))) > 0){
            const char * cmd = read_buf->cmd;
            if (strcmp(cmd, CMD_BYE) == 0) { //close command
                break;
            }else if (strcmp(cmd, CMD_CHECKFILE) == 0) {  //check command if file is exist
                server_response_check_file(server_sock, read_buf->file_name, write_buf);
            }else if (strcmp(cmd, CMD_DOWNLOAD) == 0) { // download command to download specific part
                server_upload_file(server_sock, read_buf->file_name, read_buf->start_prt, read_buf->end_prt,write_buf);
            }
            
            memset(read_buf,0,sizeof(struct client_package));
        }

        close(server_sock);
    }
    
    return 0;
}
