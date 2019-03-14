#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <regex.h>
#include <sys/stat.h>
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

int hasFile(const char * fname) {
    return access(fname, F_OK) != -1 ? 1 : 0;
}

int isNumber(const char * connectnum) {
    int p = atoi(connectnum);
    if(p == 0) return p;
    return 1;
}

long file_length(const char * filename) {
    struct stat statbuf;
    stat(filename,&statbuf);
    long size = statbuf.st_size;
    return size;

    // other choice 
    // FILE *fp=fopen(filename,"r");
    // if(!fp) return -1;
    // fseek(fp,0L,SEEK_END);
    // int size=ftell(fp);
    // fclose(fp);
    // return size;
}

int isOnlyLettersOrNumbers(const char * name) {
    	
	regex_t regex;
	int reti;
	char msgbuf[100];

	reti = regcomp(&regex, "^[a-zA-Z0-9]*$", 0);
	if (reti) {
		failHandler("regex Error!");
	}

	reti = regexec(&regex, name, 0, NULL, 0);
	if (!reti) {
        regfree(&regex);
        return 1;
	}
	else if (reti == REG_NOMATCH) {
        regfree(&regex);
		return 0;
	}
	else {
		regerror(reti, &regex, msgbuf, sizeof(msgbuf));
		failHandler("regex Error!");
	}
    return 0;
}
