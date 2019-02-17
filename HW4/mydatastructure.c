#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mydatastructure.h"

#define ADD_SIZE 5

server_list_t * initServerListArray() {
	server_list_t * arr = malloc(sizeof(server_list_t));
	arr->size = ADD_SIZE;
	arr->occupied = 0;
	arr->arrayList = calloc(ADD_SIZE,sizeof(struct sockaddr_in *));
	return arr;
}

void addServerItem(server_list_t * a, struct sockaddr_in * n) {
	if (a->size == a->occupied) {
        int n_size = ADD_SIZE + a->size;
		a->arrayList = realloc(a->arrayList, n_size * sizeof(struct sockaddr_in *));
		a->size = n_size;
	}
	a->arrayList[a->occupied] = n;
	a->occupied++;
}

struct sockaddr_in * getServerItem(server_list_t * a, int idx) {
	if (idx >= a->occupied) {
		return NULL;
	}
	return *(a->arrayList + idx);
}

void removeServerItem(server_list_t * a, int idx) {
	if(idx < 0 || idx >= a->occupied) {
		return;
	}
	int t;
	for(t = idx; t < a->occupied-1; t++) {
		a->arrayList[t] = a->arrayList[t+1];
	}
	a->occupied--;
}

void deallocServerList(server_list_t * a) {
	int t;
	struct sockaddr_in * n;
	for(t = 0; t < a->occupied; t++) {
		n = getServerItem(a, t);
		free(n);
	}
	free(a);
}

job_list_t * initJobArray(int job_size) {
	job_list_t * arr = malloc(sizeof(job_list_t));
	memset(arr, 0,  sizeof(job_list_t));
	arr->size = job_size;
	arr->occupied = 0;
	arr->job_array = calloc(job_size,sizeof(job_item_t *));
	// memset(arr->job_array, 0, job_size * sizeof(job_item_t *));
	return arr;
}

job_item_t * getJobItem(job_list_t * a, int idx) {
	return *(a->job_array + idx);
}
void addJobItem(job_list_t * a, job_item_t * item) {
	a->job_array[a->occupied] = item;
	a->occupied++;
}

void removeJobItem(job_list_t * a, int idx) {
	if(idx < 0 || idx >= a->occupied) {
		return;
	}
	int t;
	for(t = idx; t < a->occupied-1; t++) {
		a->job_array[t] = a->job_array[t+1];
	}
	a->occupied--;
}

void deallocJobList(job_list_t * a) {
	int t;
	job_item_t * n;
	for(t = 0; t < a->size; t++) {
		n = getJobItem(a, t);
		free(n);
	}
	free(a);
}

void pushJobItem(job_list_t * a, job_item_t * item) {
	addJobItem(a, item);
}

job_item_t * popJobItem(job_list_t * a) {
	if(a->occupied == 0) return NULL;
	job_item_t * n = getJobItem(a, a->occupied-1);
	removeJobItem(a, a->occupied-1);
	return n;
}

job_item_t * peekJobItem(job_list_t * a) {
	if(a->occupied == 0) return NULL;
	return getJobItem(a, a->occupied-1);
}