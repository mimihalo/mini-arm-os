#include "mstring.h"

char *strcpy(char *dest, const char *src)
{
	char *tmp = dest;

	while ((*dest++ = *src++) != '\0')
		/* nothing */;
	return tmp;
}

int strcmp(const char *cs, const char *ct)
{
	unsigned char c1, c2;

	while (1) {
		c1 = *cs++;
		c2 = *ct++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
	}
	return 0;
}

char *strtok(char *s, const char *delim)
{
	static char *lasts;
	register int ch;

	if (s == 0)
		s = lasts;
	do {
		if ((ch = *s++) == '\0')
			return 0;
	} while (strchr(delim, ch));
	--s;
	lasts = s + strcspn(s, delim);
	if (*lasts != 0)
		*lasts++ = 0;
	return s;
}

char *strchr(const char *s, int c)
{
	for (; *s != (char)c; ++s)
		if (*s == '\0')
			return 0;
	return (char *)s;
}

unsigned int strcspn(const char *s, const char *reject)
{
	const char *p;
	const char *r;
	unsigned int count = 0;

	for (p = s; *p != '\0'; ++p) {
		for (r = reject; *r != '\0'; ++r) {
			if (*p == *r)
				return count;
		}
		++count;
	}
	return count;
}

int strncmp(const char *cs, const char *ct, unsigned int count)
 {
         unsigned char c1, c2;
 
         while (count) {
                 c1 = *cs++;
                 c2 = *ct++;
                 if (c1 != c2)
                         return c1 < c2 ? -1 : 1;
                 if (!c1)
                         break;
                 count--;
         }
         return 0;
 }