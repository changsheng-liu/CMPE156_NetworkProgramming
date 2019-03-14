#ifndef _MY_PROTOCOL_H_
#define _MY_PROTOCOL_H_

#define BUFFER_SIZE 1024

#define CMD_CONFIRM_JOIN "a:c::"
#define CMD_REJECTION_JOIN "a:r::"
#define CMD_JOIN "a"
#define CMD_WAIT "w"
#define CMD_QUIT "q"
#define CMD_LIST "l" 
#define CMD_CONNECT "c"
#define CLIENT_NAME_LENGTH 16

#define wrong_cmd_msg  "Undefined command!\n"
#define talk_self_msg   "Cannot connect with your self!\n"
#define no_such_client_msg "This client is not available!\n"
#define no_available_client_msg "There are no available clients for now!\n"


char * format_join_cmd(char * my_name, int * cmd_length); //a:my_name:
//response: nothing
char * format_wait_cmd(char * my_name, char * ip, int port, int * cmd_length); //w:my_name:ip:port:
//response: nothing
char * format_quit_cmd(char * my_name, int * cmd_length); //q:my_name:
//response: nothing
char * format_list_cmd(char * my_name, int * cmd_length); //l:my_name:
//response: l:: or l:client1:client2::
char * format_connect_cmd(char * my_name, char * peer_name, int * cmd_length); //c:my_name:peer_name:
//response: c:: or c:peername:peerip:peerport:

int check_user_name_length(char * name);

#endif
