
#ifndef _MYSOCK_H_
#define _MYSOCK_H_

int build_server_socket(const int port, const char * failMessage);
int build_client_socket(const char * ip_addr,const int port, const char * failMessage);

 #endif