
#ifndef _MYDS_H_
#define _MYDS_H_

#include <netinet/in.h>
#include <arpa/inet.h>
typedef struct{
	struct sockaddr_in ** arrayList;
	int size;
	int occupied;
}server_list_t;

server_list_t * initServerListArray();
void addServerItem(server_list_t * a, struct sockaddr_in * n);
struct sockaddr_in * getServerItem(server_list_t * a, int idx);
void removeServerItem(server_list_t * a, int idx);
void deallocServerList(server_list_t * a);

#endif