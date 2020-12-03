#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/sysmacros.h>

#define BUFSIZE     100u
#define INPUT_CHECK '1'
#define INPUT_RESET '2'
#define INPUT_LAST  '3'

static int get_number(FILE * ptr, int * number);

int main(void)
{
    char buf[BUFSIZE];
    char str[BUFSIZE];
    FILE * fd;

    struct stat sb;
    char input_user = 1;
    char c;
    int data;

    printf("Witaj!\n");

    lstat("/proc/mydev", &sb);

    while(input_user)
    {
        fd = fopen("/proc/mydev", "a+r");

        if (fd != NULL)
        {
            fseek(fd, 0u ,SEEK_SET);
        }

        printf("Podaj rodzaj akcji do wykonania wciskajac numer. Inny przycisk kończy dzialanie. \n"
                "[1]: Aktualna wartosc licznika\n"
                "[2]: Zresetuj licznik\n"
                "[3]: Data ostatniego resetu\n"
                "Wybrana wartosc: ");
        scanf("%c",&input_user);

        while ((c = getchar()) != '\n' && c != EOF);

        if (input_user == INPUT_CHECK)
        {
            if ( get_number(fd, &data) )
            {
                printf("\n\nAktualna wartosc licznika = %d\n\n", data);
            }
        }
        else if (input_user == INPUT_RESET)
        {
            fputs("Set zero", fd);
            fputc('\0',fd);
            printf("\n\nResetowanie licznika\n\n");
        }
        else if (input_user == INPUT_LAST)
        {
            fgets(str, BUFSIZE, fd);              /* row with counter value*/
            if (NULL != fgets(str, BUFSIZE, fd))
            {
                printf("\n\n%s\n\n", str);
            }
        }
        else
        {
            input_user = 0;
        }

        fclose(fd);
    }

    return 0;
}



static int get_number(FILE * ptr, int * number)
{
    char c;
    char buffer[100];
    char * end;
    int i=0;
    int success = 0u;

    while ( ((c = fgetc(ptr)) != '\n') && (c != EOF) && (i<100) )
    {
        if  ((c >= '0') && (c <= '9') )
        {
            buffer[i++] = c;
        }
    }

    if ( i > 0)
    {
        success = 1u;
        *number = strtol(buffer,&end,10);
    }

    return success;
}






