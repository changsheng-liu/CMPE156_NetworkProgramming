
#ifndef _UTIL_H_
#define _UTIL_H_

int isValidIP(const char * );
int isValidPort(const char * );
int hasFile(const char * );
int isNumber(const char *);
void failHandler(const char * );
long file_length(const char * filename);
int isOnlyLettersOrNumbers(const char * name);

#endif


