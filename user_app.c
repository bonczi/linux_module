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


#define NAME_DATA "counter = "

static int get_number(FILE * ptr, int * number);

int main(void)
{
	char buf[100];
	char str[100];
	FILE * fd;

	struct stat sb;

	int input_user, c;
	int data;

	printf("Witaj!\n");

	lstat("/proc/mydev", &sb);

	while(1)
	{
		fd = fopen("/proc/mydev", "a+r");

		if (fd != NULL)
		{
			fseek(fd, 0 ,SEEK_SET);
		}

		printf("Podaj rodzaj akcji do wykonania wciskajac numer. Inny przycisk kończy dzialanie. \n"
				"[1]: Aktualna wartosc licznika\n"
				"[2]: Zresetuj licznik\n"
				"[3]: Data ostatniego resetu\n"
				"Wybrana wartosc: ");
		scanf("%d",&input_user);

		while ((c = getchar()) != '\n' && c != EOF);

		if (input_user == 1u)
		{
			if ( get_number(fd, &data) )
			{
				printf("\nAktualna wartosc licznika = %d\n", data);
			}


		}
		else if (input_user == 2u)
		{
			fputs("Set zero", fd);
			fputc('\0',fd);
		}
		else if (input_user == 3u)
		{
			fgets(str, 100, fd);
			if (NULL != fgets(str, 100, fd))
			{
				printf("\n%s\n\n", str);
			}

		    printf("Last status change:       %s", ctime(&sb.st_ctime));
		    printf("Last file access:         %s", ctime(&sb.st_atime));
		    printf("Last file modification:   %s", ctime(&sb.st_mtime));

		}
		else
		{
            break;
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



