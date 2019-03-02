# README

## Files

1. util.h + util.c: some assistance methods to format output or handle exceptions.
2. proxy.c: main file of the proxy. It contains all business related code and provides a main method as entrance.
3. dynamicarray.h + dynamicarray.c: Provide a struct and related APIs as a dynamic array struct to store forbidden list.

## How to run

after make, change to ./bin, and then run `./proxy <port> sites.txt`.

## Design of Project

### workflow

First the program would process the sites.txt to store all forbidden sites in a dynamic array.
Once the proxy build a socket and listening the port, main thread keeps to accept with a queue of 20 length. Once the handshake is accepted, the main thread would open a new thread dispatch model passed the accepted socket as an argument to the thread function. Within the thread:

1. keeps reading from the browser socket, once it received "\r\n\r\n", thread begins to parse the reqest.
2. If the request meets any requirements not to forward from homework manual, like forbidden sites, not implements methods or illegal host, the proxy would response with some error code and back to step 1.
3. If the request could be forward, the proxy needs to:
   1. find browser ip;
   2. find proxy ip;
   3. add forward message following with the request;
   4. analysis the host and get the remote server ip;
   5. build a new socket and connect to remote server;
   6. send the request;
   7. wait the response and log the first package.
4. If the any read or write has error, that means the either side has been closed, so the proxy would close the pair of the sockets and kill the thread.

### error handling: this section is for the errors have been handled

1. If there are not Host: in the request, return it as bad request.
2. If the host is wrong, proxy will return bad request.
3. If the request is HTTP/1.1 it will be default as persistent connect; if it is HTTP/1.0 and has "keep-alive" tag, it will also be persistent connect. Or it will not be persistent.
4. Set up a 60 seconds time out.
5. For persistent connect, it will detect every read and write from either side every time. Once it failed or return less than 0, it will close two sides.
6. Some other connect problems.
7. Gethostbyname make have some problems in the Timeshare GCC. I solve the problem.

#### Potential Problems

1. sometimes the socket close too quick and not send all response.
2. telnet send continuesly request as the persistent but in some situation not response.
3. sometimes kill the proxy and socket would not be released.
4. sometimes logging length is not correct.
5. If the request has something after "\r\n\r\n", they will be removed.
6. If the host name too long, the proxy will have problems.
7. In the persistent connect, the proxy cannot handle with some other hosts.
8. Only test by curl and telnet, not test real browser too much.