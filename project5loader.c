#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
    This helper function converts a hex string (like "1000")
    into an unsigned integer.

    We use this when reading addresses inside T and E records.
*/
static unsigned int parse_hex(const char *s) {
  unsigned int value = 0;

  while (*s) {
    char c = *s++;

    if  (isspace((unsigned char)c)) break;

        value <<= 4;  /* shift by 1 hex digit = 4 bits */

        if (c >= '0' && c <= '9')
            value |= (unsigned int)(c - '0');
        else if (c >= 'A' && c <= 'F')
            value |= (unsigned int)(c - 'A' + 10);
        else if (c >= 'a' && c <= 'f')
            value |= (unsigned int)(c - 'a' + 10);
        else {
            printf("error: invalid hex digit '%c'\n", c);
            exit(1);
        }
    }

    return value;
}


