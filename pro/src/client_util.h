
#ifndef _CLIENT_UTIL_H_
#define _CLIENT_UTIL_H_

typedef enum CLIENT_STATE 
{
    CLIENT_STATE_NORMAL = 0, 
    CLIENT_STATE_WAITING = 1, 
    CLIENT_STATE_TALKING = 2
}client_state_t;

//peer talk related functions
void print_received_msg(int socket);

//precheck 
void check_param(int argc, char* argv[]);

//precheck commands
int should_command_work_with_state(char * cmd, client_state_t state);

//protocol join related functions
int check_name_conflict(char * my_name, int socket);

//protocol list related functions
void process_waiting_list(int server_fd);

//send peer message function
void send_msg_to_peer(int peer_fd, char * msg, char * my_name);

#endif