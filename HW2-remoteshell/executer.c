
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <string.h>
#include "executer.h"
#include "mysocket.h"

int execute(const char * cmd, int socket) {
    FILE * fp;

    char buf[1024] = {0};
    char redir[100] = {0};
    strcpy(redir, cmd);
    strcat(redir," 2>&1");

    printf("%s\n", redir);
    if((fp = popen(redir, "r")) == NULL) {
        perror("Fail to execute!\n");
        exit(-1);
    }

    int length = 0;

    while(fgets(buf, sizeof(buf), fp) != NULL) {
        
        length += strlen(buf);

        write(socket,buf,strlen(buf));

    }
    int status = pclose(fp);

    return length;
}


// int main(int argc, char const *argv[])
// {
//     int init = 1000;
//     char * response = malloc(sizeof(char) * init);
//     memset(response, 0, sizeof(char) * init);
//     execute("pwd", response, &init);
//     printf("%s======\n", response);

//     // printf("\r\n\r\n");
//     // execute("date");
//     // printf("\r\n\r\n");
//     // execute("df -h");
//     // printf("\r\n\r\n");
//     // execute("du");
//     // printf("\r\n\r\n");
//     // execute("echo $PATH");
//     // printf("\r\n\r\n");
//     // execute("last");
//     // printf("\r\n\r\n");
//     // execute("ps aux");
//     // printf("\r\n\r\n");
//     // execute("set");
//     // printf("\r\n\r\n");
//     // execute("tty");
//     // printf("\r\n\r\n");
//     // execute("uname -a");
//     // printf("\r\n\r\n");
//     // execute("uptime");
//     // printf("\r\n\r\n");
//     // execute("who");
//     // printf("\r\n\r\n");   
//     return 0;
// }
