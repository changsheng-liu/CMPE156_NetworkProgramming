#include "myprotocol.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

char * format_join_cmd(char * my_name, int * cmd_length) { 
    //a:my_name:
    *cmd_length = strlen(my_name)+4;
    char * buf = malloc(*cmd_length);
    bzero(buf, *cmd_length);
    sprintf(buf, "a:%s:", my_name);
    return buf;
}

char * format_wait_cmd(char * my_name, char * ip, int port, int * cmd_length){
    //w:my_name:ip:port:
    char port_s[8];
    sprintf(port_s, "%d", port);
    *cmd_length = strlen(my_name)+strlen(ip)+strlen(port_s)+6;
    char * buf = malloc(*cmd_length);
    bzero(buf, *cmd_length);
    sprintf(buf, "w:%s:%s:%s:", my_name, ip, port_s);
    return buf;
}

char * format_quit_cmd(char * my_name, int * cmd_length){ 
    //q:my_name:
    *cmd_length = strlen(my_name)+4;
    char * buf = malloc(*cmd_length);
    bzero(buf, *cmd_length);
    sprintf(buf, "q:%s:", my_name);
    return buf;
}

char * format_list_cmd(char * my_name, int * cmd_length){
    //l:my_name:
    *cmd_length = strlen(my_name)+4;
    char * buf = malloc(*cmd_length);
    bzero(buf, *cmd_length);
    sprintf(buf, "l:%s:", my_name);
    return buf;
}

char * format_connect_cmd(char * my_name, char * peer_name, int * cmd_length){
    //c:my_name:peer_name:
    *cmd_length = strlen(my_name)+strlen(peer_name)+5;
    char * buf = malloc(*cmd_length);
    bzero(buf, *cmd_length);
    sprintf(buf, "c:%s:%s:", my_name,peer_name);
    return buf;
} 
