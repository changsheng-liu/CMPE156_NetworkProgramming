# README / REPORT

## Files

1. client.c: build a client process as client
2. server.c: build a server process as server
3. mysocket.h + mysocket.c: construct a self define socket protocol, including protocol and buidling sockets methods
4. util.h + util.c: some assistance methods to format output or handle exceptions.

## How to run

after make, change to ./bin and run some different servers like `./server 6640` and then run `./client server-info.txt 3 ../src/client.c` seperately.

## Design of Protocol

### Potential Problems

1. If server shutdown, the client won't reconnect to another server to download the part again. And this might cause combine file fail.
2. There are two write buffer connect to one socket. So there might be some issues if client or server has some not concensus speed.
3. The different ip limits within 1024. If there are more than 1024 ip addresses, the client will ignore and only focus on the first 1024.
4. There might be some memory leek.
5. There are too many threads initialized at first time. Most of them are immediately closed once they found out they do not have legal socket file descriptor.
6. Since there are no shared resources, there are no locks or condition variables in threads.

### Protocol Design

#### The protocol define

1. message structure. There are two types of messages, `client_package` is from client and `server_package` is from server. All messages from one side should have the same structure, but could have different commands.
2. Command. There are three kinds commands. "CHK" is for check if the file is available from server, "DWN" is to download to file, "BYE" is to close the socket.
3. protocol function. Provide client check server file function, client begin download file function, server response if file is existed function and server upload file function. When check the file, if the file exists, then server should response the file size. If the client are downloading file, the client should define from which byte to which byte to download.

#### server workflow

1. build connection, keep listening.
2. once accept one socket, keep check the input command from client, and do the correct response.

#### client workflow

1. check if the arguments are legal.
2. read server-info.txt, and build a linked list to store all information. Each node in the linked list is a pair of ip and port, and each node point to next one.
3. Based on the number of ip, user required download part to create all sockets until the less number of requirment is meeted. If there is no available connection, exit.
4. Pick up the first socket in the array to send check file command. If there is no such file, close all sockets and exit.
5. Create 1024 threads for each of ip. If the ip is not valid or the socket is not valid, thread exits; if the connection is good, calculate assigned size and try to download that part, same the file as `output.(number)`, here `(number)` is the serial number of parts, like 0, 1, 2...
6. Wait all threads finish download to join back to main thread.
7. Combine all output file into `result.txt`

#### error handling

If there are no available sockets at the beginning, the client will exit. If there are any threads or file open issues, the program will exit. If the parameters are illegal, the program will exit. If the sockets connection has issues, the program will exit.