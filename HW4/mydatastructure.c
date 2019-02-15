#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mydatastructure.h"

#define ADD_SIZE 5

server_list_t * initServerListArray() {
	server_list_t * arr = malloc(sizeof(server_list_t));
	arr->size = ADD_SIZE;
	arr->occupied = 0;
	arr->arrayList = calloc(ADD_SIZE,sizeof(server_info_t *));
	return arr;
}

void addServerItem(server_list_t * a, server_info_t * n) {
	if (a->size == a->occupied) {
        int n_size = ADD_SIZE + a->size;
		a->arrayList = realloc(a->arrayList, n_size * sizeof(server_info_t *));
		a->size = n_size;
	}
	a->arrayList[a->occupied] = n;
	a->occupied++;
}

server_info_t * getServerItem(server_list_t * a, int idx) {
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
	server_info_t * n;
	for(t = 0; t < a->occupied; t++) {
		n = getServerItem(a, t);
		free(n);
	}
	free(a);
}