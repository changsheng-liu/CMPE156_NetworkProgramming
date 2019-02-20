# Report

## Files

1. client.c: build a client process as client
2. server.c: build a server process as server
3. mysocket.h + mysocket.c: construct a self define socket protocol, including protocol and buidling sockets methods
4. util.h + util.c: some assistance methods to format output or handle exceptions.
5. executer.h + executer.c: handle all commends into Linux kernel and send to server the sh output.

## How to run

after make, change to ./bin and run `./server 8888` and `./client 127.0.0.1 8888` seperately.

## Design of Protocol

### application layer overview

send struct with a chunk of 1024 bytes, and two integer value. chunk is for the message contents, one integer value indicates how actually large the chunk is, the other is to show if this package is the last one of a stream. Comparing with the length of chunk less than maximum buffer size and is_end flag is 1 could both show that the message is end. Then read loop should break. Even the message is short, either host should send a package with empty chunk and is_end is 1 to should the end.

### application layer handshake involving

mysocket provides some methods to build the sockets. First, `init_socket` to create socket, then `init_address` to create address. These two are both for server and client.
Then server should run `server_socket_bind_listen` to establish listening socket with the address, and in a loop to run `server_socket_accept` so that server could keep create communication socket for clients.
For client, just run `client_socket_connect` to establish a socket.

Once user send `exit`, client closed. 

### message format

```c
#define BUFFER_SIZE     1024
struct my_socket_package{
    int length; // for message length
    int is_end; // indicate this is the last package or not
    char message[BUFFER_SIZE];
};
```

notice that commend from client should not be more than 1024 bytes and could not send in more than one package. 

### error handling

1. illegal commend. Before send into `popen`, I edited the commend by adding ` 2>&1` so that stderr could be into stdout to client.
2. handle ctrl-c. client catch ctrl-c signal to close socket.
3. timeout to run.

### internal design

`socket.c` provides all socket related function, including building socket connections and protocol format.
`client.c` keeps a loop to keep readline user input, send the commend to server and catch the response. Here the trick is to catch stop signal for ctrl-c and to handle where is the package end. 
`server.c` keeps big loop the accept client and a smaller loop to wait commend from server, then send the commend to executer.
`executer.c` wraps all functions to talk with kernel and send out the results. 
