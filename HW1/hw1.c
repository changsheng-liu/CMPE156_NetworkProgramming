#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <arpa/inet.h>

int main(int argc, char **argv) {
    // part 1: analysis input
    if(argc < 3 || argc > 5) {
        fprintf(stderr,"Please input correct url!\n");
        return -1;
    }
    char * t_ipaddr = argv[1];

    if(argc == 5 && strcmp(argv[4],"-h") != 0){
        fprintf(stderr,"Please enter input correct command, \"-h\" at the end as head request!\n");
        return -1;
    }
    int iplen = strlen(t_ipaddr);
    char ipaddr[16] = "";
    strcpy(ipaddr,t_ipaddr);
    int need_head = 0;
    int port = 80;
    char target[2048] = "";
    char host[2048] = "";
    if(argc == 5){
        strcpy(host, argv[2]);
        strcpy(target, argv[3]);
        need_head = 1;
    }else{
        char temp[4096] = "";
        strcpy(temp, argv[2]);
        strcpy(host, strtok(temp, ":"));
        port = atoi(strtok(NULL, "/"));
        strcpy(target, strtok(NULL, ""));
    }
    char request[4096] = "";
    if(need_head == 0){
        sprintf(request, "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n",target,host);
    }else{
        sprintf(request, "HEAD /%s HTTP/1.1\r\nHost: %s\r\n\r\n",target,host);
    }

    // part 2: form HTTP request
    int fd_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_socket < 0){
        fprintf(stderr,"Fail: cannot create socket!\n");
        return -1;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ipaddr);
    addr.sin_port = htons(port);

    if (connect(fd_socket, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        fprintf(stderr,"Fail: cannot make connection!\n");
        return -1;
    }
    if (write(fd_socket, request, sizeof(request)) < 0) {
        fprintf(stderr,"Fail: cannot send request!\n");
        return -1;
    }

    // part 3: response
    int ret;
    char buf[1024];
    if(need_head > 0){
        while((ret = read(fd_socket, buf, sizeof(buf)-1)) > 0){
            buf[ret] = 0x00;
            printf("%s",buf); //https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.bpxbd00/rtrea.htm
        }
    }else {
        FILE * fp;
        
        if((fp = fopen("output.dat", "w+")) == NULL) {
            fprintf(stderr,"Fail: cannot open target file!\n");
            return -1;
        }
        while((ret = read(fd_socket, buf, sizeof(buf)-1)) > 0){
            buf[ret] = 0x00;
            fprintf(fp, "%s", buf);
            fflush(fp);   
        }

        fclose(fp);
    }   
    close(fd_socket);
    return 0;
}