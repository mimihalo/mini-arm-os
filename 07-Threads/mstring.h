#ifndef MSTRING_H
#define MSTRING_H

char *strcpy(char *dest, const char *src);
int strcmp(const char *cs, const char *ct);
char *strtok(char *s, const char *delim);
char *strchr(const char *s, int c);
unsigned int strcspn(const char *s, const char *reject);
int strncmp(const char *cs, const char *ct, unsigned int count);

#endif