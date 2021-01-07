#define INCL_BASE
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc,char *argv[])
{
    ULONG freq=0,dur=0;
    if (argc != 3)
    {
       printf("bla [freq] [dur]\n");
       return 1;
    }
    printf("Hallo!\n");
    freq = strtoul(argv[1],NULL,10);
    dur  = strtoul(argv[2],NULL,10);
    if (!freq || !dur)
    {
       printf("invalid arguments\n");
       return 1;
    }
    DosBeep(freq,dur);
    return 0;
}

