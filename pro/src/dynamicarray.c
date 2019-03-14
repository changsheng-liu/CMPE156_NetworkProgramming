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
	int t_len = 2;
	bzero(temp, temp_size);
	strcpy(temp,"l:");
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
	strcat(temp,":");
	*bufsize = t_len+1;
	return temp;
}

client_name_list_t * initNameArray() {
	client_name_list_t * arr = malloc(sizeof(client_name_list_t));
	memset(arr, 0, sizeof(client_name_list_t));
	arr->occupied = 0;
    arr->size = ADD_SIZE;
	arr->array = calloc(ADD_SIZE, sizeof(char *));
	memset(arr->array, 0, ADD_SIZE * sizeof(char *));
	return arr;
}

void addNameItem(client_name_list_t * a, char * n) {
	if (a->size == a->occupied) {
        int n_size = ADD_SIZE + a->size;
		a->array = realloc(a->array, n_size * sizeof(char *));
		a->size = n_size;
	}
	a->array[a->occupied] = n;
	a->occupied++;
}
 
int hasNameItem(client_name_list_t * a, char * n) {
	for(int i = 0; i < a->occupied; i++) {
		char * item = *(a->array + i);
		if(strcmp(n, item) == 0){
			return 1;
		}
	}
	return 0;
}

void removeNameItem(client_name_list_t * a, char * n) {
	int idx = -1;
	for(int i = 0; i < a->occupied; i++) {
		char * item = *(a->array + i);
		if(strcmp(n, item) == 0){
			idx = i;
		}
	}
	if(idx == -1){
		return;
	}
	for(int i = idx; i < a->occupied-1; i++) {
		a->array[i] = a->array[i+1];
	}
	a->occupied--;
}

void printNameList(client_name_list_t * a) {
	for(int i = 0; i < a->occupied; i++) {
		char * item = *(a->array + i);
	}
}