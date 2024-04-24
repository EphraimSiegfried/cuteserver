//https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
#include <ctype.h>
#include <string.h>

char *trim(char *str) {
    char *end;

    while (isspace((unsigned char) *str)) str++;

    if (*str == 0)
        return str;

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char) *end)) end--;

    end[1] = '\0';

    return str;
}
