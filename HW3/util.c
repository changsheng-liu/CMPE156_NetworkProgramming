#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <regex.h>
#include "util.h"

int isValidIP(const char * ipaddr) {
    const char * ip_pattern = "^([0-9]{1,3}\\.|\\*\\.){3}([0-9]{1,3}|\\*){1}$";
	regex_t reg;
    int ret = regcomp(&reg, ip_pattern, REG_EXTENDED);
    if(ret > 0){
        return 0;
    }
    int status = regexec(&reg,ipaddr,0,NULL,0);
    if(status == REG_NOMATCH) {
        regfree(&reg);
        return 0;
    }
    regfree(&reg);

    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipaddr, &(sa.sin_addr));
    return result > 0?1:0;
}

int isValidPort(const char * port) {
    int p = atoi(port);
    if(p == 0) return p;
    return 1;
}

void failHandler(const char * failMessage) {
    fprintf(stderr, "%s\n" , failMessage);
    exit(-1);
}