#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <ctype.h>
#include <regex.h>
#include <time.h>

int isValidIP(char * ipaddr) {
    const char * ip_pattern = "^([0-9]{1,3}\\.|\\*\\.){3}([0-9]{1,3}|\\*){1}$";
	regex_t reg;
    int ret = regcomp(&reg, ip_pattern, REG_EXTENDED);
    if(ret > 0){
        return 0;
    }
    int status = regexec(&reg,ipaddr,0,NULL,0);
    if(status == REG_NOMATCH) {
        regfree(&reg);
        return 0;
    }
    regfree(&reg);

    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipaddr, &(sa.sin_addr));
    return result > 0?1:0;
}

int main(int argc, char **argv) {
    // part 1: analysis input
    if(argc < 3 || argc > 4) {
        fprintf(stderr,"Please input as instruction!\n");
        return -1;
    }

    if(argc == 4 && strcmp(argv[3],"-h") != 0){
        fprintf(stderr,"Please input correct command, \"-h\" at the end for head request!\n");
        return -1;
    }

    char * t_ipaddr = argv[1];
    if(isValidIP(t_ipaddr) == 0) {
        fprintf(stderr,"Please input legal IP address!\n");
        return -1;
    }

    //ip address
    char ipaddr[16] = "";
    strcpy(ipaddr,t_ipaddr);

    //get or head
    int need_head = 0;
    if(argc == 4){
        need_head = 1;
    }

    //host, port and target
    if(strlen(argv[2]) > 4096){
        fprintf(stderr,"URL is too long!\n");
        return -1;
    }
    char temp[4096] = "";
    char * httphead = strstr(argv[2],"http://");
    if(httphead != NULL) {
        strcpy(temp, argv[2]+7);
    }else{
        strcpy(temp, argv[2]);
    }
    char host[512] = "";
    int port = 80;
    char target[3584] = "";

    char * isPort = strstr(temp, ":");

    char * t;
    if(isPort == NULL){
        if(strstr(temp, "/") != NULL){
            strcpy(host, strtok(temp,"/"));
            t = strtok(NULL, "");
            if(t != NULL){
                strcpy(target, t);
            }
        }else{
            strcpy(host, temp);
        }
    }else{
        if(strlen(isPort) > 2 && isdigit(isPort[1])){
            strcpy(host, strtok(temp, ":"));
            if(strstr(temp, "/") != NULL){
                port = atoi(strtok(NULL, "/"));
                strcpy(target, strtok(NULL, ""));
            }else{
                port = atoi(strtok(NULL, ""));
            }
        }else{
            fprintf(stderr, "Illegal port number!\n");
            return -1;
        }
    }

    char request[4128] = "";
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
            printf("%s",buf);
        }
    }else {
        FILE * fp;
        time_t start =  clock(), finish;
        if((fp = fopen("output.dat", "w+")) == NULL) {
            fprintf(stderr,"Fail: cannot open target file!\n");
            return -1;
        }
        while((ret = read(fd_socket, buf, sizeof(buf)-1)) > 0){
            buf[ret] = 0x00;
            fprintf(fp, "%s", buf);
            fflush(fp); 
            // extra task: ttl
            finish = clock();
            double diff = difftime(start,finish);
            if(diff > 240) {
                break;
            }
        }

        fclose(fp);
        
        // extra task: remove head
        if((fp = fopen("output.dat", "r+")) == NULL) {
            fprintf(stderr,"Fail: cannot remove head in output file!\n");
            return -1;
        }
        FILE* fp2;
        if((fp2 = fopen("output2.dat", "w+")) == NULL) {
            fprintf(stderr,"Fail: cannot remove head in output file!\n");
            return -1;
        }
        char line[4096];
        int isBody = 0;
        while(!feof(fp)) {
		    fgets(line,4096,fp);
            if(isBody == 1){
                fprintf(fp2, "%s", line);
                fflush(fp2); 
            }
            if(isBody == 0 && strstr(line, "\r\n") != NULL && line[0] == '\r') {
                isBody = 1;
            }
	    }
        fclose(fp2);
        fclose(fp);
    }   
    close(fd_socket);
    return 0;
}