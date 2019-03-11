#include "myprotocol.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char * format_join_cmd(char * my_name, int * cmd_length) { 
    //a:my_name:

}

char * format_wait_cmd(char * my_name, char * ip, int port, int * cmd_length){
    //w:my_name:ip:port:
}

char * format_quit_cmd(char * my_name, int * cmd_length){ 
    //q:my_name:
}

char * format_list_cmd(char * my_name, int * cmd_length){
    //l:my_name:
}

char * format_connect_cmd(char * my_name, char * peer_name, int * cmd_length){
    //c:my_name:peer_name:
} 
