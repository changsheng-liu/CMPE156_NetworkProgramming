#include "dynamicarray.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ADD_SIZE 5

client_list_t * initArray(){
    client_list_t * arr = malloc(sizeof(client_list_t));
	memset(arr, 0, sizeof(client_list_t));
	arr->occupied = 0;
    arr->size = ADD_SIZE;
	arr->array = calloc(ADD_SIZE, sizeof(client_t *));
	memset(arr->array, 0, ADD_SIZE * sizeof(client_t *));
	return arr;
}

void addItem(client_list_t * a, client_t * item) {
    if (a->size == a->occupied) {
        int n_size = ADD_SIZE + a->size;
		a->array = realloc(a->array, n_size * sizeof(client_t *));
		a->size = n_size;
	}
	a->array[a->occupied] = item;
	a->occupied++;
}

client_t * getItem(client_list_t * a, int idx) {
    if(idx >= a->occupied) {
        return NULL;
    }
    return * (a->array + idx);
}

void removeItem(client_list_t * a, int idx) {
    if(idx < 0 || idx >= a->occupied) {
		return;
	}
	int t;
	for(t = idx; t < a->occupied-1; t++) {
		a->array[t] = a->array[t+1];
	}
	a->occupied--;
}

client_t * popItem(client_list_t * a, int idx) {
    if(idx < 0 || idx >= a->occupied) return NULL;
	client_t * n = getItem(a, idx);
	removeItem(a, idx);
	return n;
}

int findItem(client_list_t * a, char * name) {
	for(int i = 0; i < a->occupied; i++) {
		client_t * item = getItem(a, i);
		printf("######%ld\n",strlen(item->client));
		printf("^^^^^%ld\n",strlen(name));
		int rt = strcmp(item->client, name);
		if(rt == 0){
			return i;
		}
	}
	return -1;
}

char * printList(client_list_t * a, int * bufsize) {
	int temp_size = 1024;
	char *temp = malloc(temp_size);
	int t_len = 1;
	bzero(temp, temp_size);
	strcpy(temp,":");
	for(int i = 0; i < a->occupied; i++) {
		client_t * item = getItem(a, i);
		t_len = t_len + strlen(item->client)+1;
		if(t_len > temp_size) {
			temp_size = temp_size * 2;
			temp = realloc(temp, temp_size);
			strcat(temp,item->client);
		}else{
			strcat(temp,item->client);
		}
		strcat(temp,":");
	}
	*bufsize = t_len;
	return temp;
}