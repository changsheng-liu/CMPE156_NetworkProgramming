# README

## Files

1. client.c: client main progress
2. server.c: server main progress
3. myprotocol.h: build the protocol, independent with programming languages or programs.
4. util.h + util.c: some assistance methods to format output or handle exceptions.
5. dynamicarray.h + dynamicarray.c: present private define data structure, and provide related API to use. Support dynamic arrays.
6. client_util.h + client_util.c: client related functions and definitions to handle response, like independent IO funtions.

## How to run

after make, change to ./bin and run some different servers like `./server 12345` and then run `./client 127.0.0.1 12345 <username>` seperately.

## Design of application-layer Protocol

### Command design

The protocol is based on TCP and have 6 basic commands from client to server:

1. add: when a user starts to run and try to join into the chatting room, this command is for server to allow client with a unique user id. The command is like this `a:my_name:`, the response could either be `a:c::` to allow or `a:r::` to reject.
2. wait: when a user enter into WAIT mode, it will send to server like this: `w:my_name:port:`. The server should store the waiting ip by check peer ip and the port number identified by this command and the waiting name. There is no confirm response from server.
3. list: when a user enter into INFO mode, it will send to server like this: `w:my_name:`. The client should response all waiting users by sending `l:client1:client2::` if there are 2 clients or `l::` if there is none.
4. quit: when the user wants to quit and exit the program, it will send to server like this: `q:my_name:`. Server should handle the message but not response.
5. connect: when a user tried to directly connect with some other client, it should send to server this command first: `c:my_name:peer_name:`. The server should either response `c::` if the peer client is not available or  `c:peername:peerip:peerport:` to be used.
6. leave-wait: if a client is waiting and decide to quit the waiting mode to INFO mode, the client should send the request `r:my_name:`. The server should not response anything but handle it silently.

## Program Design

### server workflow

1. Open one socket, then bind with the well known port, keep waiting clients to connect.
2. Once get a connection, create a thread and process the client within the thread.
3. Response / handle each command from client.
4. When receive quit command, exit the thread.

### server structure

Server has two dynamic arrays to maintain the whole chatting room. One is only store user names to make sure each user has a unique name, a new client with a duplicate name would be rejected; The other is to store all waiting users. Each entry has its user name, ip and port number. Quit command would affect both two arrays.

### client workflow

0. There are three modes for a client: NORMAL, WAITING and TALKING. NORMAL can become WAITING by `/wait`, or became TALKING by `/connect`. WAITING can become TALKING by connected by other users; either TALKING or WAITING can become NORMAL by `^C`. TALKING cannot become WAITING directly.
1. check if the arguments are legal.
2. check if the user name is legal. The user name can only have numbers or letters within 15 characters.
3. In main thread, connect to the server, send add command to make sure not conflict user names. If so, give the hint and exit the program.
4. In main thread, keep reading from user input. If user input a command, do the response functions. If user input a regular message, make sure it is in TALKING mode and send it out.
5. When user type `/wait`, the program create a new thread to keep waiting `accept()` response by `select()`. Once it is accepted, create another thread to keep receiving the peer message.
6. When user type `/connect`, send the query to server for peer ip address. Once get the response, user create a thread to keep receiving the peer message.
7. Once received `^C`, program should check the state mode first and change it to NORMAL. And change some flags or show some message hint to trip close sockets.
8. When user type `/quit`, close all sockets first and give the hint to user and exit the program.

### error handling: this section is for the errors have been handled

__client:__

1. User name is too long.
2. User name have conflict.
3. User name have chars other than letters and numbers.
4. User input incorrect command.
5. User input incomplete commend like `/connect` instead of `/connect peer_name`.
6. No waiting users.
7. Connect to incorrect users.
8. Input message when it is not in talking mode.
9. Waiting users are too much. One buffer is not enough.
10. Cannot catch interrupt when waiting.
11. When user get a peer message, it should be shown in the next line.
12. When user input `^C`, too much hints are shown.

__server:__

1. response correctly.
2. Conflict users.
3. Users are too much.
4. Waiting users has conflicts.

#### Potential Problems

1. cannot handle too long user name(>15 char)
2. cannot handle too long message.
3. When `^C` to NORMAL mode, the command is duplicated shown on the screen.