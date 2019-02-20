# README

## Files

1. client.c: build a client process as client
2. server.c: build a server process as server
3. myprotocol.h: build the protocol, independent with programming languages or programs.
4. util.h + util.c: some assistance methods to format output or handle exceptions.
5. mydatastructure.h + mydatastructure.c: present private define data structure, and provide related API to use. Support dynamic arrays.

## How to run

after make, change to ./bin and run some different servers like `./server 6640` and then run `./client server-info.txt 3 ../src/client.c` seperately.

## Design of Protocol

### application-layer protocol

There are three commands in the protocol, handshake, check file, and download file. Server will response any commands in a struct `server_reponse_t` with different contents.

1. Handshake command: `"HSK"`. This command is to check if the server is alive. Server should response immediately to show liveness. In `cmd` slot of `server_reponse_t`, the server will set "HSK".
2. Check file command: `"CHK <filename>"`. This command is to check if the server has that file named "filename". Server can response within struct `server_reponse_t` with `cmd` slot as "CHK", `file_content` slot as the queried file name, and set `have_file_flag` slot as 1 if the file exists or -1 if not.
3. Download file command: `"DWN <filename> <start-byte-position> <end-byte-position>"`. Once receives this command, server would response one or more `server_reponse_t` packages. In each package, the `cmd` slot would be "DWN", and the `file_content` would be the contents of file between start and end position.

### handshake

Since this program is based on UDP, at the beginning of the client program running, the client would check if the servers in the info list would be alive. That is the only time the client tried to handshake with servers. The following communications would based on socket time out for the failure of servers.

## Program Design

### server workflow

1. Open one socket, then bind with the well known port, keep receiving client packages.
2. Once get a command, check which the command is.
3. For handshake or check file command, response with the well-known port socket.
4. For download file command, open a thread. Within that thread, create a new socket with a new port number, and upload the file part by the new port. After finish uploading, close the socket and end the thread.

### client workflow

1. check if the arguments are legal.
2. read server-info.txt, and build a dynamic array to store all information.
3. In main thread, check the liveness of all servers by sending handshake command. __NOTE:__ this step may take a little bit longer time.
4. Once there are available servers, use main thread to send check file command to the first living server. If file doesn't exists, the client will remind the user and exists; if it exists, the client would know the size of the target file by the server response.
5. Client would cut the task into chunk number of subtasks. Each task will be assignned to a independent thread. The task will include the begin and end part of the file which should be downloaded.
6. Each thread will create a independent socket and send the download command to server. Server will response by one or more packages. Each thread will make sure all subtaskes will be done. If in this process the server dies, the thread will put the task to a unfinish task list and exit the thread.
7. Once all threads finish and join into main thread, the main thread would combine all subfiles and store the result into dest directory.

### error handling: this section is for the errors have been handled

client:

1. Input not as reqired or not make sense. 
2. No living server in the server info list at the beginning.
3. Server send half job then dead.
4. chunk as too much.

server:

1. download part overflow of the file.
2. no existing file.
3. client dead during downloading.

#### Potential Problems

1. If the server dead during the last thread downloading the last chunk, client may stuck.
2. two client at the same directory working at the same time may comflict.
3. It may take a little bit long time to check all servers' liveness.
4. UDP package may out of order to arrived.
5. UDP package may cover the buffer if the receive side not process the buffer in time.
6. memory leak if I do not maintain heap. heap corrupted if I maintain heap. The version I commited is memory leak version.
7. new socket in the pthread of server cannot receive command, but only send.