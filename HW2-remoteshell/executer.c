
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "executer.h"

void execute(const char * cmd, int * socket, int * ret_size) {
    FILE * fp;
    const int pad = 2048;

    char buf[pad] = {0};
    if((fp = popen(cmd, "r")) == NULL) {
        perror("Fail to execute!\n");
        exit(-1);
    }

    char * total;
    int empty = pad;
    int length = 0;
    total = (char *)malloc(sizeof(char) * pad);

    while(fgets(buf, sizeof(buf), fp) != NULL) {
        while(strlen(buf) >= empty) {
            total = (char *)realloc(total, length+pad);
            empty += pad;
        }
        strcpy(total+length, buf);
        length += strlen(buf);
        empty -= strlen(buf);

        write(*socket,buf,strlen(buf));

    }
    *ret_size = length;
    pclose(fp);
}

// void t(const char * cmd) {
//     FILE * fp;
//     const int pad = 2048;

//     char buf[pad] = {0};
//     if((fp = popen(cmd, "r")) == NULL) {
//         perror("Fail to execute!\n");
//         exit(-1);
//     }

//     int mmm;
//     while((mmm = fread(buf, sizeof(char), sizeof(buf), fp)) != NULL) {
        
//         printf("%s", buf);
//     }

//     pclose(fp);
// }

// int main(int argc, char const *argv[])
// {
//         t("set");
//         return 0;
// }


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
