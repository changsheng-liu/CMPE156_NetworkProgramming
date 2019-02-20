#define BUFFER_SIZE     1024
#define COMMAND_SIZE 4

//protocol define 
//download command: "DWN <filename> <startbyteposition> <endbyteposition>"
#define CMD_DOWNLOAD "DWN" 
//check file command: "CHK <filename>"
#define CMD_CHECKFILE "CHK"
//check if server is alive: "HSK"
#define CMD_HANDSHAKE "HSK"

typedef struct{
    long file_length;
    long sequence_num;
    int have_file_flag;
    char cmd[COMMAND_SIZE];
    char file_content[BUFFER_SIZE];
}server_response_t;
