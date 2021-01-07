#define INCL_BASE
#include <os2.h>

#include <stdio.h>

int main(int argc,char *argv[])
{
    printf("Hallo!\n");
    DosBeep(800,1000);
    return 0;
}