
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

typedef struct{ 
	int job_id;
	const char * file_name;
	long file_start;
	long file_end;
}job_item_t;

typedef struct{
	job_item_t ** job_array;
	int size;
	int occupied;
}job_list_t;

job_list_t * initJobArray(int job_size);
job_item_t * getJobItem(job_list_t * a, int idx);
void addJobItem(job_list_t * a, job_item_t * item);
void removeJobItem(job_list_t * a, int idx);
void deallocJobList(job_list_t * a);

#endif