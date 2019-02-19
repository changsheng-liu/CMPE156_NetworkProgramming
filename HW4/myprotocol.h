#define BUFFER_SIZE     1024
#define COMMAND_SIZE 4

//protocol define 
//download command: "DWN <filename> <startbyteposition> <endbyteposition>"
#define CMD_DOWNLOAD "DWN" 
//check file command: "CHK <filename>"
#define CMD_CHECKFILE "CHK"
//tell server to start a new port to start download: "STR"
#define CMD_START "STR"
//tell server finish download job and allow to thread exit: "FIN"
#define CMD_FIN "FIN"
//check if server is alive: "HSK"
#define CMD_HANDSHAKE "HSK"

typedef struct{
    long file_length;
    long download_start;
    int have_file_flag;
    int download_port;
    char cmd[COMMAND_SIZE];
    char file_content[BUFFER_SIZE];
}server_response_t;
