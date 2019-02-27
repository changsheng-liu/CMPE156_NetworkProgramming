#ifndef _DYNAMICARRAY_H_
#define _DYNAMICARRAY_H_

typedef struct{
	char ** sitesArray;
	int occupied;
}forbidden_sites_t;
  
forbidden_sites_t * initSitesArray();
void addSiteItem(forbidden_sites_t * a, char * n);
char * getSiteItem(forbidden_sites_t * a, int idx);
void deallocSitesArray(forbidden_sites_t * a);

#endif