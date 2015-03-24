#include "string.h"
#include "stdio.h"

void cold_code()
{
    printf("%s\n", "This is cold code");
}

void very_cold_code()
{
    printf("%s\n", "This is very cold code");
}

void strcopy_is_the_devil(char *str)
{
    char input[3];
      strcpy(input, str);
}

void main(int argc, char *argv[])
{
    strcopy_is_the_devil(argv[1]);
    printf("Exiting cleanly.\n");
}
