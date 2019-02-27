#include "dynamicarray.h"
#include <stdlib.h>
#include <string.h>

forbidden_sites_t * initSitesArray() {
    forbidden_sites_t * arr = malloc(sizeof(forbidden_sites_t));
	memset(arr, 0, sizeof(forbidden_sites_t));
	arr->occupied = 0;
	arr->sitesArray = calloc(1, sizeof(char *));
	memset(arr->sitesArray, 0, 1 * sizeof(char *));
	return arr;
}

void addSiteItem(forbidden_sites_t * a, char * n) {
    a->sitesArray = realloc(a->sitesArray, (a->occupied + 1) * sizeof(char *));
    char * item = malloc(128 * sizeof(char));
    bzero(item, 128);
    strcpy(item, n);
    a->sitesArray[a->occupied] = item;
    a->occupied++;
}

char * getSiteItem(forbidden_sites_t * a, int idx) {
    if(idx >= a->occupied) {
        return NULL;
    }
    return * (a->sitesArray+idx);
}

void deallocSitesArray(forbidden_sites_t * a) {
    int t;
	char * n;
	for(t = 0; t < a->occupied; t++) {
		n = getSiteItem(a, t);
		free(n);
	}
	free(a);
}