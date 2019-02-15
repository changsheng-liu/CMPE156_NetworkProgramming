#include <unistd.h>
#include <string.h>
#include "stdlib.h"
#include "util.h"
#include "mysocket.h"

int init_socket(const char * failMessage){
    int tcpsocket = socket(AF_INET, SOCK_STREAM, 0);
    if(tcpsocket < 0) {
        failHandler(failMessage);
    }
    return tcpsocket;
}

struct sockaddr_in init_address(in_addr_t address, const int port) {
    struct sockaddr_in addr; 
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = address;
    addr.sin_port = htons(port);
    return addr;
}

void server_socket_bind_listen(int lis_sock, struct sockaddr * addr, int max_clients, const char * failMessage) {
    if(bind(lis_sock, addr, sizeof(*addr)) < 0){
        failHandler(failMessage);
    }
    if(listen(lis_sock, max_clients) < 0){
        failHandler(failMessage);
    }
}

int server_socket_accept(int lis_sock, struct sockaddr * addr, const char * failMessage) {
    int addr_len = sizeof(*addr);
    int server_sock = accept(lis_sock, addr, (socklen_t *) &addr_len);
    if(server_sock < 0) {
        failHandler(failMessage);
    }

    return server_sock;
}

int client_socket_connect(int cli_sock ,struct sockaddr * addr, const char * failMessage) {
    if(connect(cli_sock, addr, sizeof(*addr)) < 0){
        return -1;
    }
    return 1;
}

void client_check_file(int conn_sock , const char * filename, struct client_package * write_buf) {
    memset(write_buf, 0, sizeof(struct client_package));
    strcpy(write_buf->cmd, CMD_CHECKFILE);
    strcpy(write_buf->file_name,filename);
    write(conn_sock,write_buf,sizeof(struct client_package));
}

void client_request_file(int conn_sock, const char * filename, long start, long end, struct client_package * write_buf) {
    memset(write_buf, 0, sizeof(struct client_package));
    strcpy(write_buf->cmd, CMD_DOWNLOAD);
    strcpy(write_buf->file_name,filename);
    write_buf->start_prt = start;
    write_buf->end_prt = end;
    write(conn_sock,write_buf,sizeof(struct client_package));
}

void server_response_check_file(int conn_sock, const char * filename, struct server_package * write_buf) {
    memset(write_buf, 0, sizeof(struct server_package));
    strcpy(write_buf->cmd, CMD_CHECKFILE);

    if (hasFile(filename)) {
        write_buf->have_file_flag = 1;
        write_buf->file_length = file_length(filename);
    }else {
        write_buf->have_file_flag = -1;
    }
    write(conn_sock,write_buf,sizeof(struct server_package));
}

void server_upload_file(int conn_sock, const char * filename, long start, long end, struct server_package * write_buf) {
    memset(write_buf, 0, sizeof(struct server_package));
    strcpy(write_buf->cmd, CMD_DOWNLOAD);
    
    FILE *fp; 
	if((fp = fopen(filename,"r")) == NULL) { 
		exit(0);
	} 

	fseek(fp, start, SEEK_SET);
	long cur = start;
	char data[BUFFER_SIZE];
	memset(data, 0x00, sizeof(char)*BUFFER_SIZE);

	int rt;
	while(end - cur > BUFFER_SIZE-1) {
		rt = fread(data,1,BUFFER_SIZE-1,  fp);
        data[rt] = 0x00;
		strcpy(write_buf->file_content, data);
        write_buf->content_is_end = -1;
        write_buf->content_length = rt;
		cur = cur + rt;
        write(conn_sock,write_buf, sizeof(struct server_package));
		memset(data, 0x00, sizeof(char)*BUFFER_SIZE);
	}
	rt = fread(data, 1, (end-cur+1), fp);
	data[rt] = 0x00;
	strcpy(write_buf->file_content, data);
    write_buf->content_is_end = 1;
    write_buf->content_length = rt;
    write(conn_sock,write_buf, sizeof(struct server_package));

	fclose(fp);
}

void socket_exit(int conn_sock, struct client_package * write_buf) {
    memset(write_buf, 0, sizeof(struct client_package));
    strcpy(write_buf->cmd, CMD_BYE);
    write(conn_sock,write_buf,sizeof(struct client_package));
}