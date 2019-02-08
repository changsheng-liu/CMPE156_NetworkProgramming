
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "executer.h"
#include "mysocket.h"

void execute(char * cmd, int socket) {
    FILE * fp;

    char redir[BUFFER_SIZE+5] = {0};
    strcpy(redir, cmd);
    strcat(redir," 2>&1");

    char popen_buf[BUFFER_SIZE] = {0};

    if((fp = popen(redir, "r")) == NULL) {
        perror("Fail to execute!\n");
        exit(-1);
    }
    
    // FILE * temp;
    // if((temp = fopen("cmd", "w+")) == NULL) {
    //     fprintf(stderr,"Fail: cannot open target file!\n");
    //     pclose(fp);
    //     return;
    // }
    struct my_socket_package * write_buf = malloc(sizeof(struct my_socket_package));

    while(fgets(popen_buf,sizeof(popen_buf),fp)!=0) {
        // fprintf(temp,"%s", popen_buf);
        // printf("%s", popen_buf);

        memset(write_buf, 0, sizeof(struct my_socket_package));
        write_buf->length = 1;
        write_buf->is_end = 0;
        strcpy(write_buf->message, popen_buf);
        write(socket,write_buf,sizeof(struct my_socket_package));
       


		memset(popen_buf, 0, sizeof(popen_buf));

    }
    // fclose(temp);  
    pclose(fp);
        write_buf->length = 1;
        write_buf->is_end = 1;
        strcpy(write_buf->message, "");
        write(socket,write_buf,sizeof(struct my_socket_package));
       
    // if((temp = fopen("cmd", "r+")) == NULL) {
    //     fprintf(stderr,"Fail: cannot open target file!\n");
    //     return;
    // }

    // int res;

    // while (1) {
    //     res = fread(popen_buf, 1, sizeof(popen_buf), temp);

    //     memset(write_buf, 0, sizeof(struct my_socket_package));
    //     write_buf->length = res;
    //     write_buf->is_end = res<sizeof(popen_buf)?1:0;
    //     strcpy(write_buf->message, popen_buf);
    //     write(socket,write_buf,sizeof(struct my_socket_package));
        
    //     memset(popen_buf, 0, sizeof(popen_buf));

    //     if(res<sizeof(popen_buf)) break;
    // }
    // fclose(temp);
    // fclose(fopen("cmd", "w"));
    // remove("cmd");
}