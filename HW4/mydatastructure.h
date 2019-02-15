
#ifndef _MYDS_H_
#define _MYDS_H_

typedef struct{
	int port;
	char IP[16];
}server_info_t;

typedef struct{
	server_info_t ** arrayList;
	int size;
	int occupied;
}server_list_t;

server_list_t * initServerListArray();
void addServerItem(server_list_t * a, server_info_t * n);
server_info_t * getServerItem(server_list_t * a, int idx);
void removeServerItem(server_list_t * a, int idx);
void deallocServerList(server_list_t * a);

#endif