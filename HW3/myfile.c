#include <stdio.h>
#include <sys/stat.h>

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