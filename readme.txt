This is a source code sample for replacing the DosBeep and Dos16Beep functions to make it sound on the multimedia audio device instead of the PC speaker.

This sample replaces:
-	Dos32Beep (DOSCALL.286)
-	Dos16Beep  (DOSCALLS.50)

Effectively it works like this:

1) ANY application (that is, either an EXE or a DLL that this EXE binds to) that uses DosBeep can be modified to instead use the DosBeep that is implemented in NEWCALLS.DLL. That also includes any DLLs delivered with OS/2.

2) What you need to do to this EXE or DLL is to overwrite the import references that import from DOSCALLS (where ordinal 286 points to the genuine DosBeep function as implemented in the kernel: it uses the PIT to create a sound via an attached PC built-in LOW-FI speaker) and instead tell the EXE or DLL to import from NEWCALLS (which uses your soundcard to play a generated PCM sine wave stream).

3) Because there is a good chance that a number of DOSCALLS routines are bound to the exe and not only DosBeep, what NEWCALLS does is to forward all unchanged functions directly from DOSCALLS to the EXE or DLL. With the one exception of "DosBeep" which NEWCALLS replaces with its own implementation (and it binds to the genuine implementation and calls that where necessary).

Let's say you have an EXE called MyApp.exe that links to a couple of functions implemented in the kernel, say, DosOpen, DosClose, DosRead, DosWrite and also DosBeep.

Run this command:
  dllrname.exe /Q /N MyApp.exe DOSCALLS=NEWCALLS

After doing so, MyApp.exe will invoke the genuine kernel functions DosOpen,DosClose, DosRead,DosWrite in DOSCALLS (because those are forwarded) but it will call the implementation of DosBeep in NEWCALLS as that has been replaced.

You will have to do the "dllrname" action on every EXE and DLL where you want to have a DosBeep call replaced by the implementation in NEWCALLS.DLL. "dllrname.exe" is a tool that comes with VAC but there are free tools and serve the very same purpose.

https://hobbes.nmsu.edu/download/pub/os2/dev/util/dllrname.zip
https://hobbes.nmsu.edu/download/pub/os2/util/system/renmodul2_0_0.zip

NEWCALLS.DLL has to go somewhere in your LIBPATH or BEGINLIBPATH or ENDLIBPATH, just like for any other DLL.

Just a hint: You can only do this module renaming (DOSCALLS=NEWCALLS) if the modules have the very same name length (DOSCALLS and NEWCALLS have the same number of letters). That is a design limitation. Additionally, under OS/2, DLL names are limited to 8 characters (excluding file extension).

Additional note: I have implemented DosBeep so that PM and also VIO applications can use it without any restrictions. I even went through some hoola hoops so that error message boxes can be displayed even from a VIO app.

==PROCEDURE==
You need to modify the binary (EXE or DLL) so it can use the NEWCALLS.DLL (New DOSBeep) file instead of DOSCALL.DLL. Remember to backup the EXE or DLL before doing this.

Follow the next procedure:

First validate if the EXE uses DosBeep. If you have an exe called MyApp.Exe, then do this:
 exehdr /V MyApp.exe | find "imp"
It will list all the imports. If you find something like this:
  REL OFF(32)  0014       imp DOSCALLS.286
Then you know that this exe uses DosBeep.

Now modify the file by running:
  dllrname /N /Q MyApp.exe DOSCALLS=NEWCALLS
You can rerun:
  exehdr /V MyApp.exe | find "imp"
Now it will show this:
  REL OFF(32)  0014       imp NEWCALLS.286

Now you will know that from now on, the new DosBeep Implementation will be called. It works the very same way to patch a DLL.

==SAMPLE==
There is this simple app to help you test this, file bla.c:

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

and build it with "build.cmd".

After that you can try:
 dllrname /N /Q bla.exe DOSCALLS=NEWCALLS /* this does the interesting part */

When you now run bla.exe, you can hear the Beep via my Speakers driven by Uniaud or via the Speakers connected via Audio USB, whatever device I choose as the default WAV device (the default WAV device will be used to play the sound).

By the way: use my USBAUDIO.ZIP package to have everything to work with USB audio. And it also comes with a DLL that extends the Multimedia Setup Object to allow you to simply switch the default WAV device with a mouse click. Of course that works across the board, not only for the USB audio device but also for the UNIAUD device.

==REFERENCE==
This sample is inspired by the NEWCALLS sample of "Hook code to circumvent built-in methods of OS/2" by Peter Fitzsimmons.

http://www.edm2.com/common/snippets/newc2.zip

==LICENSE==
BSD 3-Clauses

==AUTHOR==
Lars Erdmann

==LINKS==
http://www.edm2.com/common/snippets/newc2.zip
https://hobbes.nmsu.edu/download/pub/os2/dev/util/dllrname.zip
https://hobbes.nmsu.edu/download/pub/os2/util/system/renmodul2_0_0.zip

